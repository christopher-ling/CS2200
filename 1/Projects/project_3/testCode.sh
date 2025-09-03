#! /bin/bash


while getopts "rfl" opt; do
    case $opt in
        r)
		make clean
		make
        	echo Testing RANDOM eviction algorithm
		rm traces/astar.trace
		cp GoodTraces/astar.trace traces/
		./vm-sim -c -i traces/astar.trace -rrandom >  randomOutput.txt
	
		echo printing your implementation values
		cat randomOutput.txt | tail -n7
		echo ...............done.................
		
		
		echo printing correct implementation values
		cat outputs/astar_random.log | tail -n7
		echo ...............done.................
	
		echo diff:
		diff <(tail -n7 randomOutput.txt) <(tail -n7 outputs/astar_random.log)
		echo ...............done.................
		exit 0
		;;
        f)
		make clean
		make

		echo Testing FIFO eviction algorithm
		rm traces/astar.trace
		cp GoodTraces/astar.trace traces/
		./vm-sim -c -i traces/astar.trace -rfifo > fifoOutput.txt
	
		echo printing your implementation values
		cat fifoOutput.txt | tail -n7
		echo ...............done.................
		
		
		echo printing correct implementation values
		cat outputs/astar_fifo.log | tail -n7
		echo ...............done.................
	
		echo diff:
		diff <(tail -n7 fifoOutput.txt) <(tail -n7 outputs/astar_fifo.log)
		echo ...............done.................
		exit 0

            ;;
        l)
		make clean
		make
	
		echo Testing APPROX_LRU algorithm
		rm traces/astar.trace
		cp GoodTraces/astar.trace traces/
		./vm-sim -c -i traces/astar.trace -rlru > lruOutput.txt

		echo printing your implementation values
		cat lruOutput.txt | tail -n7
		echo ...............done.................


		echo printing correct implementation values
		cat outputs/astar_lru.log | tail -n7
		echo ...............done.................

		echo diff:
		diff <(tail -n7 lruOutput.txt) <(tail -n7 outputs/astar_lru.log)
		echo ...............done.................
		exit 0
            ;;
    esac
done

