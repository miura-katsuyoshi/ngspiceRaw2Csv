#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#define min(A, B) (((A)<(B)) ? (A) : (B))

#define BUFSIZE (256)
#define LABEL_SIZE (16)

void chop(char buf[], int bufsize)
{
	int i;
	for (i = strnlen(buf, bufsize) - 1;
	     i >= 0 && buf[i] != ' ' && buf[i] != '\n' && buf[i] != '\t'; i--) {
	}
	buf[i] = '\0';
}

void wordncpy(char *dest, char *src, int bufsize)
{
	int i, j;
	for (i = 0;
	     i < bufsize - 1 && src[i] != '\0' && (src[i] == ' '
						   || src[i] == '\n'
						   || src[i] == '\t'); i++) {
	}
	for (j = 0;
	     i < bufsize - 1 && src[i] != '\0' && src[i] != ' '
	     && src[i] != '\n' && src[i] != '\t'; i++, j++) {
		dest[j] = src[i];
	}
	dest[j] = '\0';
	return;
}

void lowercase(char *str, int bufsize)
{
	int i;
	for(i=0; i<bufsize-1 && str[i] != '\0'; i++)
	{
		if(str[i] >= 'A' && str[i] <= 'Z') str[i] = str[i] - ('A' - 'a');
	}
}

int main(int argc, char **argv)
{
	char buf[BUFSIZE];
	char *endptr;
	int n_vars, n_points, n_output_vars;
	double *data;
	char *label;
	int i, j, k;
	char dbuf[sizeof(double)];
	int *output_vars;

	/* Convert arguments to lowercase */
	for(i=1; i<argc; i++) lowercase(argv[i], LABEL_SIZE);

	/*
	 * Header format
	 *
	 * Title: * circuit description
	 * Date: Mon Jun 13 23:38:21  2016
	 * Plotname: Transient Analysis
	 * Flags: real
	 * No. Variables: 2
	 * No. Points: 1956    
	 * Variables:
	 *    0       time    time
	 *    1       v(n2110)        voltage
	 * Binary:
	 */

	/* Title: * circuit description */
	if(NULL == fgets(buf, BUFSIZE, stdin)) {
		perror("Unexpected EOF or I/O error");
		exit(1);
	}

	/* Date: Mon Jun 13 23:38:21  2016 */
	if(NULL == fgets(buf, BUFSIZE, stdin)) {
		perror("Unexpected EOF or I/O error");
		exit(1);
	}

	/* Plotname: Transient Analysis */
	if(NULL == fgets(buf, BUFSIZE, stdin)) {
		perror("Unexpected EOF or I/O error");
		exit(1);
	}

	/* Flags: real */
	if(NULL == fgets(buf, BUFSIZE, stdin)) {
		perror("Unexpected EOF or I/O error");
		exit(1);
	}
	if (strncmp("Flags: real\n", buf, BUFSIZE)) {
		fprintf(stderr, "Format error: %s", buf);
		exit(1);
	}

	/* No. Variables: 2 */
	if(NULL == fgets(buf, BUFSIZE, stdin)) {
		perror("Unexpected EOF or I/O error");
		exit(1);
	}
	if (strncmp("No. Variables: ", buf, min(15, BUFSIZE))) {
		fprintf(stderr, "Format error: %s", buf);
		exit(1);
	}
	n_vars = strtol(buf + 15, &endptr, 10);
	if (buf + 15 == endptr) {
		fprintf(stderr, "Format error: %s", buf);
		exit(1);
	}

	label = (char *)calloc(n_vars, LABEL_SIZE);
	if (NULL == label) {
		perror("memory allocation failed");
		exit(1);
	}

	/* No. Points: 1956 */
	if(NULL == fgets(buf, BUFSIZE, stdin)) {
		perror("Unexpected EOF or I/O error");
		exit(1);
	}
	if (strncmp("No. Points: ", buf, min(12, BUFSIZE))) {
		fprintf(stderr, "Format error: %s", buf);
		exit(1);
	}
	n_points = strtol(buf + 12, &endptr, 10);
	if (buf + 12 == endptr) {
		fprintf(stderr, "Format error: %s", buf);
		exit(1);
	}

	/* Variables: */
	if(NULL == fgets(buf, BUFSIZE, stdin)) {
		perror("Unexpected EOF or I/O error");
		exit(1);
	}
	if (strncmp("Variables:\n", buf, BUFSIZE)) {
		fprintf(stderr, "Format error: %s", buf);
		exit(1);
	}

	// Read variable labels
	for (i = 0; i < n_vars; i++) {
		if(NULL == fgets(buf, BUFSIZE, stdin)) {
			perror("Unexpected EOF or I/O error");
			exit(1);
		}
		if (i != strtol(buf, &endptr, 10)) {
			fprintf(stderr, "Invalid variable No.: %s", buf);
			exit(1);
		}
		wordncpy(label + LABEL_SIZE * i, endptr, LABEL_SIZE);
		lowercase(label + LABEL_SIZE * i, LABEL_SIZE);
	}

	/* Select output fields */
	n_output_vars = n_vars;
	if(argc > 1) n_output_vars = argc - 1;
	output_vars = (int *)calloc(n_output_vars, sizeof(int));
	if(NULL == output_vars) {
		perror("memory allocation failed");
		exit(1);
	}
	if(argc==1) {
		/* All fields are printed (default). */
		for(i=0; i<n_output_vars; i++) output_vars[i] = i;
	} else {
		/* Only specified fields are printed. */
		for(i=0; i<n_output_vars; i++) {
			for(j=0; j<n_vars; j++) {
				if(0 == strncmp(label + LABEL_SIZE * j, argv[i + 1], LABEL_SIZE)) break;
			}
			if(j<n_vars) output_vars[i] = j;
			else {
				fputs("unknown variable: ", stderr);
				fputs(argv[i+1], stderr);
				fputc('\n', stderr);
				output_vars[i] = -1;
			}
		}
	}

	fputs("# ", stdout);
	for(i=0; i < n_output_vars; i++) {
		if(output_vars[i]<0) {
			fputs("unknown:", stdout);
			fputs(argv[i+1], stdout);
		} else {
			fputs(label + LABEL_SIZE * output_vars[i], stdout);
		}
		if (i < n_output_vars - 1) {
			putchar(',');
		} else {
			putchar('\n');
		}
	}

	/* Binary: */
	if(NULL == fgets(buf, BUFSIZE, stdin)) {
		perror("Unexpected EOF or I/O error");
		exit(1);
	}
	if (strncmp("Binary:\n", buf, BUFSIZE)) {
		fprintf(stderr, "Format error: %s", buf);
		exit(1);
	}

	data = (double *)calloc(n_vars, sizeof(double));
	if (NULL == data) {
		perror("memory allocation failed");
		exit(1);
	}

	for (i = 0; i < n_points; i++) {
		for (j = 0; j < n_vars; j++) {
			for (k = 0; k < sizeof(double); k++) {
				dbuf[k] = getchar();
			}
			data[j] = *(double *)dbuf;
		}
		for(j = 0; j < n_output_vars; j++) {
			if(output_vars[j] >= 0) printf("%.14e", data[output_vars[j]]);
			if (j < n_output_vars - 1) {
				putchar(',');
			} else {
				putchar('\n');
			}
		}
	}

	return 0;
}
