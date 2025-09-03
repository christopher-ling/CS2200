#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include "queue.h"
#include "network.h"
#include "rtp.h"

/**
 * PLESE ENTER YOUR INFORMATION BELOW TO RECEIVE MANUAL GRADING CREDITS
 * Name: Christopher Ling
 * GTID: 903813567
 * Spring 2025
 */

typedef struct message
{
    char *buffer;
    int length;
} message_t;

/* ================================================================ */
/*                  H E L P E R    F U N C T I O N S                */
/* ================================================================ */

/**
 * --------------------------------- PROBLEM 1 --------------------------------------
 *
 * Convert the given buffer into an array of PACKETs and returns the array.  The
 * value of (*count) should be updated so that it contains the number of packets in
 * the array. Keep in mind that if length % MAX_PAYLOAD_LENGTH != 0, you might have
 * to manually specify the payload_length.
 *
 * Hint: can we use a heap function to make space for the packets? How many packets?
 *
 * @param buffer pointer to message buffer to be broken up packets
 * @param length length of the message buffer.
 * @param count number of packets in the returning array
 *
 * @returns array of packets
 */
packet_t *packetize(char *buffer, int length, int *count)
{

    packet_t *packets;
    int packetsTotal = length / MAX_PAYLOAD_LENGTH;

    if (length % MAX_PAYLOAD_LENGTH != 0) {
        packetsTotal++;
    }

    *count = packetsTotal;

    packets = malloc(sizeof(packet_t) * packetsTotal);
    if (!packets) {
        perror("malloc packetize");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < packetsTotal; i++) {
        int len = (i < length / MAX_PAYLOAD_LENGTH) ? MAX_PAYLOAD_LENGTH : (length % MAX_PAYLOAD_LENGTH ? length % MAX_PAYLOAD_LENGTH : MAX_PAYLOAD_LENGTH);
        packets[i].type = (i == packetsTotal - 1 ? LAST_DATA : DATA);
        packets[i].payload_length = len;

        memcpy(packets[i].payload, buffer + i * MAX_PAYLOAD_LENGTH, len);
        
        packets[i].checksum = checksum(packets[i].payload, packets[i].payload_length);
    }

    return packets;
}

/**
 * --------------------------------- PROBLEM 2 --------------------------------------
 *
 * Compute a checksum based on the data in the buffer.
 *
 * Checksum calcuation:
 * Suppose we are given a string in the form
 *      "a b c d e f g h"
 * Then swap each pair of characters such that the characters appear in the form
 *      "b a d c f e h g"
 *
 * If the length of the string is odd, the last character will remain in the same place.
 *
 * i.e. reorder the string such that each pair of characters are swapped.
 * Following this, the checksum will be equal to the sum of the ASCII values of each
 * character multiplied by it's index in the string.
 *
 * @param buffer pointer to the char buffer that the checksum is calculated from
 * @param length length of the buffer
 *
 * @returns calcuated checksum
 */
int checksum(char *buffer, int length)
{

    int output = 0;

    char temp[length];
    memcpy(temp, buffer, length);

    for (int i = 0; i + 1 < length; i += 2) {
        char c = buffer[i];
        temp[i] = buffer[i+1];
        temp[i+1] = c;
    }

    for (int i = 0; i < length; i++) {
        output += temp[i] * i;
    }

    return output;
}

/* ================================================================ */
/*                      R T P       T H R E A D S                   */
/* ================================================================ */

static void *rtp_recv_thread(void *void_ptr)
{

    rtp_connection_t *connection = (rtp_connection_t *)void_ptr;

    do
    {
        message_t *message;
        int buffer_length = 0;
        char *buffer = NULL;
        packet_t packet;

        /* Put messages in buffer until the last packet is received  */
        do
        {
            if (net_recv_packet(connection->net_connection_handle, &packet) <= 0 || packet.type == TERM)
            {
                /* remote side has disconnected */
                connection->alive = 0;
                pthread_cond_signal(&connection->recv_cond);
                pthread_cond_signal(&connection->send_cond);
                break;
            }

            /*  ----  FIXME: Part III-A ----
             *
             * 1. Check to make sure payload of packet is correct
             * 2. Send an ACK or a NACK, whichever is appropriate
             * 3. If this is the last packet in a sequence of packets
             *    and the payload was corrupted, make sure the loop
             *    does not terminate
             * 4. If the payload matches, add the payload to the buffer
             */

            /*
             *  What if the packet received is not a data packet?
             *  If it is a NACK or an ACK, the sending thread should
             *  be notified so that it can finish sending the message.
             *
             *  1. Add the necessary fields to the CONNECTION data structure
             *     in rtp.h so that the sending thread has a way to determine
             *     whether a NACK or an ACK was received
             *  2. Signal the sending thread that an ACK or a NACK has been
             *     received.
             */
            
            if (packet.type == DATA || packet.type == LAST_DATA) {
                // Packet is a DATA or LAST_DATA packet
                if (checksum(packet.payload, packet.payload_length) == packet.checksum) {
                    // Send ACK after checksum
                    packet_t ack;
                    ack.type = ACK;
                    ack.checksum = 0;
                    ack.payload_length = 0;

                    // Send relevant packets to server
                    net_send_packet(connection->net_connection_handle, &ack);
                    buffer = realloc(buffer, buffer_length + packet.payload_length);
                    if (!buffer) { perror("realloc"); exit(EXIT_FAILURE); }
                    memcpy(buffer + buffer_length, packet.payload, packet.payload_length);
                    buffer_length += packet.payload_length;
                } else {
                    // Send NACK
                    packet_t nack;
                    nack.type = NACK;
                    nack.checksum = 0;
                    nack.payload_length = 0;
                    net_send_packet(connection->net_connection_handle, &nack);
                    packet.type = DATA;
                    continue;
                }
            }

            if (packet.type == ACK || packet.type == NACK) {
                // Packet is a ACK or NACK
                pthread_mutex_lock(&connection->ack_mutex);
                connection->ack = (packet.type == ACK) ? 1 : 0;
                pthread_cond_signal(&connection->ack_cond);
                pthread_mutex_unlock(&connection->ack_mutex);
                continue;
            }
        } while (packet.type != LAST_DATA);

        if (connection->alive == 1)
        {
            /*  ----  FIXME: Part III-B ----
             *
             * Now that an entire message has been received, we need to
             * add it to the queue to provide to the rtp client.
             *
             * 1. Add message to the received queue.
             * 2. Signal the client thread that a message has been received.
             */
            message = malloc(sizeof(message_t));
            message->buffer = buffer;
            message->length = buffer_length;
            pthread_mutex_lock(&connection->recv_mutex);
            queue_add(&connection->recv_queue, message);
            pthread_mutex_unlock(&connection->recv_mutex);
            pthread_cond_signal(&connection->recv_cond);
        }
        else
            free(buffer);

    } while (connection->alive == 1);

    return NULL;
}

static void *rtp_send_thread(void *void_ptr)
{

    rtp_connection_t *connection = (rtp_connection_t *)void_ptr;
    message_t *message;
    int array_length = 0;
    int i;
    packet_t *packet_array;

    do
    {
        /* Extract the next message from the send queue */
        pthread_mutex_lock(&connection->send_mutex);
        while (queue_size(&connection->send_queue) == 0 && connection->alive == 1)
        {
            pthread_cond_wait(&connection->send_cond, &connection->send_mutex);
        }

        if (connection->alive == 0)
        {
            pthread_mutex_unlock(&connection->send_mutex);
            break;
        }

        message = queue_extract(&connection->send_queue);

        pthread_mutex_unlock(&connection->send_mutex);

        /* Packetize the message and send it */
        packet_array = packetize(message->buffer, message->length, &array_length);

        for (i = 0; i < array_length; i++)
        {

            /* Start sending the packetized messages */
            if (net_send_packet(connection->net_connection_handle, &packet_array[i]) <= 0)
            {
                /* remote side has disconnected */
                connection->alive = 0;
                break;
            }

            /*  ----FIX ME: Part III-C ----
             *
             *  1. Wait for the recv thread to notify you of when a NACK or
             *     an ACK has been received
             *  2. Check the data structure for this connection to determine
             *     if an ACK or NACK was received.  (You'll have to add the
             *     necessary fields yourself)
             *  3. If it was an ACK, continue sending the packets.
             *  4. If it was a NACK, resend the last packet
             */
            pthread_mutex_lock(&connection->ack_mutex);
            connection->ack = -1;
            while (connection->ack == -1) {
                pthread_cond_wait(&connection->ack_cond, &connection->ack_mutex);
            }
            int got = connection->ack;
            pthread_mutex_unlock(&connection->ack_mutex);
            if (got == 0) { i--; } // NACK => resend
        }

        free(packet_array);
        free(message->buffer);
        free(message);
    } while (connection->alive == 1);
    return NULL;
}

/* ================================================================ */
/*                           R T P    A P I                         */
/* ================================================================ */

static rtp_connection_t *rtp_init_connection(int net_connection_handle)
{
    rtp_connection_t *rtp_connection = malloc(sizeof(rtp_connection_t));

    if (rtp_connection == NULL)
    {
        fprintf(stderr, "Out of memory!\n");
        exit(EXIT_FAILURE);
    }

    rtp_connection->net_connection_handle = net_connection_handle;

    queue_init(&rtp_connection->recv_queue);
    queue_init(&rtp_connection->send_queue);

    pthread_mutex_init(&rtp_connection->ack_mutex, NULL);
    pthread_mutex_init(&rtp_connection->recv_mutex, NULL);
    pthread_mutex_init(&rtp_connection->send_mutex, NULL);
    pthread_cond_init(&rtp_connection->ack_cond, NULL);
    pthread_cond_init(&rtp_connection->recv_cond, NULL);
    pthread_cond_init(&rtp_connection->send_cond, NULL);

    rtp_connection->alive = 1;

    pthread_create(&rtp_connection->recv_thread, NULL, rtp_recv_thread,
                   (void *)rtp_connection);
    pthread_create(&rtp_connection->send_thread, NULL, rtp_send_thread,
                   (void *)rtp_connection);

    return rtp_connection;
}

rtp_connection_t *rtp_connect(char *host, int port)
{

    int net_connection_handle;

    if ((net_connection_handle = net_connect(host, port)) < 1)
        return NULL;

    return (rtp_init_connection(net_connection_handle));
}

int rtp_disconnect(rtp_connection_t *connection)
{

    message_t *message;
    packet_t term;

    term.type = TERM;
    term.payload_length = term.checksum = 0;
    net_send_packet(connection->net_connection_handle, &term);
    connection->alive = 0;

    net_disconnect(connection->net_connection_handle);
    pthread_cond_signal(&connection->send_cond);
    pthread_cond_signal(&connection->recv_cond);
    pthread_join(connection->send_thread, NULL);
    pthread_join(connection->recv_thread, NULL);
    net_release(connection->net_connection_handle);

    /* emtpy recv queue and free allocated memory */
    while ((message = queue_extract(&connection->recv_queue)) != NULL)
    {
        free(message->buffer);
        free(message);
    }
    queue_release(&connection->recv_queue);

    /* emtpy send queue and free allocated memory */
    while ((message = queue_extract(&connection->send_queue)) != NULL)
    {
        free(message);
    }
    queue_release(&connection->send_queue);

    free(connection);

    return 1;
}

int rtp_recv_message(rtp_connection_t *connection, char **buffer, int *length)
{

    message_t *message;

    if (connection->alive == 0)
        return -1;
    /* lock */
    pthread_mutex_lock(&connection->recv_mutex);
    while (queue_size(&connection->recv_queue) == 0 && connection->alive == 1)
    {
        pthread_cond_wait(&connection->recv_cond, &connection->recv_mutex);
    }

    if (connection->alive == 0)
    {
        pthread_mutex_unlock(&connection->recv_mutex);
        return -1;
    }

    /* extract */
    message = queue_extract(&connection->recv_queue);
    *buffer = message->buffer;
    *length = message->length;
    free(message);

    /* unlock */
    pthread_mutex_unlock(&connection->recv_mutex);

    return *length;
}

int rtp_send_message(rtp_connection_t *connection, char *buffer, int length)
{

    message_t *message;

    if (connection->alive == 0)
        return -1;

    message = malloc(sizeof(message_t));
    if (message == NULL)
    {
        return -1;
    }
    message->buffer = malloc((size_t)length);
    message->length = length;

    if (message->buffer == NULL)
    {
        free(message);
        return -1;
    }

    memcpy(message->buffer, buffer, (size_t)length);

    /* lock */
    pthread_mutex_lock(&connection->send_mutex);

    /* add */
    queue_add(&(connection->send_queue), message);

    /* unlock */
    pthread_mutex_unlock(&connection->send_mutex);
    pthread_cond_signal(&connection->send_cond);
    return 1;
}