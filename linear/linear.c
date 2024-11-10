#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>

const double eps = 1e-6;

static inline double feq(double a, double b) {
    return fabs(a - b) < eps;
}

typedef struct vector {
    size_t n;
    double* data;
} vector_t;

vector_t vector_new(size_t n) {
    double* data = (double*)malloc(sizeof(double) * n);
    return (vector_t) {
        .n = n,
        .data = data
    };
}

vector_t* vector_new_array(size_t n, size_t count) {
    vector_t* vector_head = (vector_t*)malloc(
        (sizeof(vector_t) * count) + 
        (sizeof(double) * n * count)
    );
    double* data_head = (double*)(vector_head + count);
    for (size_t i = 0; i < count; i++) {
        vector_t* cur_head = vector_head + i;
        cur_head->n = n;
        cur_head->data = data_head + n * i;
    }
    for (size_t i = 0; i < n * count; i++)
        data_head[i] = 0;
    return vector_head;
}

void vector_mul(vector_t v, double c) {
    for (size_t i = 0; i < v.n; i++)
        v.data[i] *= c;
}

void vector_add_mul(vector_t v, vector_t other, double c) {
    for (size_t i = 0; i < v.n; i++)
        v.data[i] += other.data[i] * c;
}

void vector_copy(vector_t dest, vector_t src) {
    for (size_t i = 0; i < dest.n; i++)
        dest.data[i] = src.data[i];
}

void vector_print(vector_t v) {
    printf("[");
    for (size_t i = 0; i < v.n; i++) {
        printf("%.3lf", v.data[i]);
        if (i != v.n - 1)
            printf(", ");
    }
    printf("]\n");
}

typedef struct matrix {
    size_t row, col;
    vector_t* vectors;
} matrix_t;

matrix_t matrix_new(size_t row, size_t col) {
    vector_t* vectors = vector_new_array(col, row);
    return (matrix_t) {
        .row = row,
        .col = col,
        .vectors = vectors
    };
}

void matrix_eliminate(matrix_t matrix, size_t row_col) {
    for (size_t i = 0; i < row_col; i++) {
        double c = matrix.vectors[i].data[row_col] / matrix.vectors[row_col].data[row_col];
        vector_add_mul(matrix.vectors[i], matrix.vectors[row_col], -c);
    }
}

// return 1 if inserted successfully
// return 0 if vec is linear linearly dependent to vectors in matrix
// return -1 if vec do not fit other elements in matrix
int matrix_insert_gaussian(matrix_t matrix, vector_t vec) {
    for (size_t i = 0; i < matrix.row; i++) {
        if (feq(vec.data[i], 0.0)) {
            continue;
        }
        if (feq(matrix.vectors[i].data[i], 0.0)) {
            // Ensure head is always 1
            vector_mul(vec, 1.0 / vec.data[i]);
            vector_copy(matrix.vectors[i], vec);
            return 1;
        }
        vector_add_mul(vec, matrix.vectors[i], -vec.data[i]);
    }
    if (feq(vec.data[vec.n - 1], 0.0))
        return 0;
    else 
        return -1;
}

void matrix_print(matrix_t matrix) {
    printf("[[");
    for (size_t row = 0; row < matrix.row; row++) {
        if (row != 0)
            printf("[");
        for (size_t col = 0; col < matrix.vectors[row].n; col++) {
            printf("%.3lf", matrix.vectors[row].data[col]);
            if (col != matrix.vectors[row].n - 1)
                printf(", ");
        }
        printf("]");
        if (row != matrix.row - 1)
            printf(",\n ");
    }
    printf("]\n");
}

void matrix_formula_print(matrix_t matrix) {
    for (size_t row = 0; row < matrix.row; row++) {
        for (size_t col = 0; col < matrix.vectors[row].n - 1; col++) {
            printf("%.6lf x%llu", matrix.vectors[row].data[col], col + 1);
            if (col != matrix.vectors[row].n - 2)
                printf(" + ");
        }
        printf(" = ");
        printf("%.6lf\n", matrix.vectors[row].data[matrix.vectors[row].n - 1]);
    }
}

void generate_data(size_t fc, size_t vc, int seed) {
    srand(seed);
    freopen("data", "w+", stdout);
    printf("%llu %llu\n", fc, vc);
    for (size_t i = 0; i < fc; i++) {
        for (size_t j = 0; j <= vc; j++) {
            double v = (double)rand() / (double)RAND_MAX * 1000.0;
            printf("%lf ", v);
        }
        printf("\n");
    }
}

int main() {
    // generate_data(997, 1000, 114514);
    // return 0;
    // freopen("data", "r", stdin);
    // freopen("out", "w+", stdout);
    fprintf(stderr, "Enter number of equations and variable count: \n");
    size_t y, x;
    scanf("%llu%llu", &y, &x);
    matrix_t matrix = matrix_new(x, x + 1);
    for (size_t i = 0; i < y; i++) {
        vector_t vector = vector_new(x + 1);
        fprintf(stderr, "Formula %llu coefficients and constants: \n", i + 1);
        for (size_t j = 0; j <= x; j++)
            scanf("%lf", &vector.data[j]);
        int result = matrix_insert_gaussian(matrix, vector);
        if (result == 0)
            fprintf(stderr, "Linear dependent vector on %llu.\n", i + 1);
        else if (result == -1)
            fprintf(stderr, "Invalid vector on %llu.\n", i + 1);
    }
    for (size_t i = 0; i < x; i++) {
        if (!feq(matrix.vectors[i].data[i], 0.0))
            matrix_eliminate(matrix, i);
    }
    matrix_formula_print(matrix);
}