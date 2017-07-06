#ifndef MINISPHERE__MATRIX_H__INCLUDED
#define MINISPHERE__MATRIX_H__INCLUDED

typedef struct matrix matrix_t;

matrix_t*                matrix_new          (void);
matrix_t*                matrix_clone        (const matrix_t* matrix);
matrix_t*                matrix_ref          (matrix_t* matrix);
void                     matrix_free         (matrix_t* matrix);
const ALLEGRO_TRANSFORM* matrix_transform    (const matrix_t* matrix);
float*                   matrix_items        (matrix_t* matrix);
void                     matrix_compose      (matrix_t* matrix, const matrix_t* other);
void                     matrix_identity     (matrix_t* matrix);
void                     matrix_orthographic (matrix_t* matrix, float left, float right, float top, float bottom, float z_near, float z_far);
void                     matrix_perspective  (matrix_t* matrix, float left, float right, float top, float bottom, float z_near, float z_far);
void                     matrix_rotate       (matrix_t* matrix, float theta, float vx, float vy, float vz);
void                     matrix_scale        (matrix_t* matrix, float sx, float sy, float sz);
void                     matrix_translate    (matrix_t* matrix, float dx, float dy, float dz);

#endif // MINISPHERE__MATRIX_H__INCLUDED
