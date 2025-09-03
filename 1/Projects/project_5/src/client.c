#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "network.h"
#include "rtp.h"

static void printUsage()
{
    fprintf(stderr, "Usage:  rtp-client host port\n\n");
    fprintf(stderr, "   example ./rtp-client localhost 4000\n\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{

    char message_1[] = "Jr pna bayl frr n fubeg qvfgnapr nurnq, ohg jr pna frr cyragl gurer gung arrqf gb or qbar. - Nyna Ghevat";

    char message_2[] = "Gur zbfg qnatrebhf cuenfr va gur ynathntr vf, 'Jr'ir nyjnlf qbar vg guvf jnl.' - Tenpr Ubccre";

    char message_3[] = "Vg vf abg rabhtu gb unir n tbbq zvaq. Gur znva guvat vf gb hfr vg jryy. - Erar Qrfpnegrf";

    char message_4[] = "Gur orfg jnl gb cerqvpg gur shgher vf gb vairag vg. - Nyna Xnl";

    char message_5[] = "Pbzchgre fpvrapr vf ab zber nobhg pbzchgref guna nfgebabzl vf nobhg gryrfpbcrf. - Rqftre Qvwxfgen";

    char *rcv_buffer;
    int length, ret;
    rtp_connection_t *connection;

    if (argc < 3)
    {
        printUsage();
        return EXIT_FAILURE;
    }

    if ((connection = rtp_connect(argv[1], atoi(argv[2]))) == NULL)
    {
        printUsage();
        return EXIT_FAILURE;
    }

    printf("Sending quotes to a remote host to have them "
           "decoded using ROT13 cipher!\n\n");

    rtp_send_message(connection, message_1, strlen(message_1));
    ret = rtp_recv_message(connection, &rcv_buffer, &length);
    if (rcv_buffer == NULL || ret == -1)
    {
        printf("Connection reset by peer.\n");
        return EXIT_FAILURE;
    }
    printf("%.*s\n\n", length, rcv_buffer);
    free(rcv_buffer);

    rtp_send_message(connection, message_2, strlen(message_2));
    ret = rtp_recv_message(connection, &rcv_buffer, &length);
    if (rcv_buffer == NULL || ret == -1)
    {
        printf("Connection reset by peer.\n");
        return EXIT_FAILURE;
    }
    printf("%.*s\n\n", length, rcv_buffer);
    free(rcv_buffer);

    rtp_send_message(connection, message_3, strlen(message_3));
    ret = rtp_recv_message(connection, &rcv_buffer, &length);
    if (rcv_buffer == NULL || ret == -1)
    {
        printf("Connection reset by peer.\n");
        return EXIT_FAILURE;
    }
    printf("%.*s\n\n", length, rcv_buffer);
    free(rcv_buffer);

    rtp_send_message(connection, message_4, strlen(message_4));
    ret = rtp_recv_message(connection, &rcv_buffer, &length);
    if (rcv_buffer == NULL || ret == -1)
    {
        printf("Connection reset by peer.\n");
        return EXIT_FAILURE;
    }
    printf("%.*s\n\n", length, rcv_buffer);
    free(rcv_buffer);

    rtp_send_message(connection, message_5, strlen(message_5));
    ret = rtp_recv_message(connection, &rcv_buffer, &length);
    if (rcv_buffer == NULL || ret == -1)
    {
        printf("Connection reset by peer.\n");
        return EXIT_FAILURE;
    }
    printf("%.*s\n\n", length, rcv_buffer);
    free(rcv_buffer);
    rtp_disconnect(connection);
    return EXIT_SUCCESS;
}