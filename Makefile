all = ngspiceRaw2Csv

ngspiceRaw2Csv : ngspiceRaw2Csv.c
	gcc -O3 -o ngspiceRaw2Csv ngspiceRaw2Csv.c

clean:
	rm ngspiceRaw2Csv
