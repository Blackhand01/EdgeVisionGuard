#ifndef ZANT_MODEL_H
#define ZANT_MODEL_H

#ifdef __cplusplus
extern "C" {
#endif

void zant_infer(const float* input, float* output);
void zant_free_result(float* ptr);

#ifdef __cplusplus
}
#endif

#endif
