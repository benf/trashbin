#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct _matrix {
	uint8_t n, m;
	double **data;
} matrix_t;

matrix_t *create_matrix(uint8_t n, uint8_t m) {
	matrix_t *matrix = malloc(sizeof(matrix_t));

	matrix->n = n;
	matrix->m = m;

	matrix->data = calloc(matrix->n, sizeof(double *));
	for (int i = 0; i < matrix->n; ++i)
		matrix->data[i] = calloc(matrix->m, sizeof(double));
	return matrix;
}

matrix_t *identity_matrix(uint8_t n) {
	matrix_t *matrix = create_matrix(n, n);
	for (int i = 0; i < n; ++i)
			matrix->data[i][i] = 1.0;
	return matrix;
}

matrix_t *copy_matrix(matrix_t *matrix) {
	matrix_t *neu = create_matrix(matrix->n, matrix->m);

	for (int i = 0; i < matrix->n; ++i)
		for (int j = 0; j < matrix->m; ++j)
			neu->data[i][j] = matrix->data[i][j];
	return neu;
}

void free_matrix(matrix_t *matrix) {
	for (int i = 0; i < matrix->n; ++i)
		free(matrix->data[i]);
	free(matrix->data);
	free(matrix);
}
