#include "mouse_stabilizer.h"

KalmanFilter g_kalman_x = {0};
KalmanFilter g_kalman_y = {0};

void InitializeKalman(KalmanFilter* kf, float q, float r) {
    kf->x = 0.0f;
    kf->y = 0.0f;
    kf->p_x = 1.0f;
    kf->p_y = 1.0f;
    kf->q = q;
    kf->r = r;
}

float KalmanUpdate(KalmanFilter* kf, float measurement) {
    kf->p_x = kf->p_x + kf->q;
    
    float k = kf->p_x / (kf->p_x + kf->r);
    
    kf->x = kf->x + k * (measurement - kf->x);
    kf->p_x = (1 - k) * kf->p_x;
    
    return kf->x;
}

MousePoint MovingAverageFilter(const MouseStabilizer* stabilizer) {
    if (stabilizer->count == 0) {
        return (MousePoint){0, 0, 0};
    }
    
    float sum_x = 0.0f, sum_y = 0.0f;
    int samples = stabilizer->count < 5 ? stabilizer->count : 5;
    
    for (int i = 0; i < samples; i++) {
        int idx = (stabilizer->head - 1 - i + MAX_BUFFER_SIZE) % MAX_BUFFER_SIZE;
        sum_x += stabilizer->buffer[idx].x;
        sum_y += stabilizer->buffer[idx].y;
    }
    
    return (MousePoint){
        sum_x / samples,
        sum_y / samples,
        GetTickCount()
    };
}

MousePoint ExponentialFilter(const MouseStabilizer* stabilizer) {
    if (stabilizer->count == 0) {
        return (MousePoint){0, 0, 0};
    }
    
    int current_idx = (stabilizer->head - 1 + MAX_BUFFER_SIZE) % MAX_BUFFER_SIZE;
    MousePoint current = stabilizer->buffer[current_idx];
    
    if (stabilizer->count == 1) {
        return current;
    }
    
    float alpha = stabilizer->smoothing_strength;
    
    float filtered_x = alpha * current.x + (1 - alpha) * stabilizer->last_output.x;
    float filtered_y = alpha * current.y + (1 - alpha) * stabilizer->last_output.y;
    
    return (MousePoint){filtered_x, filtered_y, current.timestamp};
}

MousePoint KalmanFilter_Apply(const MouseStabilizer* stabilizer) {
    if (stabilizer->count == 0) {
        return (MousePoint){0, 0, 0};
    }
    
    int current_idx = (stabilizer->head - 1 + MAX_BUFFER_SIZE) % MAX_BUFFER_SIZE;
    MousePoint current = stabilizer->buffer[current_idx];
    
    float filtered_x = KalmanUpdate(&g_kalman_x, current.x);
    float filtered_y = KalmanUpdate(&g_kalman_y, current.y);
    
    return (MousePoint){filtered_x, filtered_y, current.timestamp};
}

MousePoint ApplyFilter(const MouseStabilizer* stabilizer) {
    MousePoint result = {0};
    
    switch (stabilizer->filter_type) {
        case FILTER_MOVING_AVERAGE:
            result = MovingAverageFilter(stabilizer);
            break;
        case FILTER_EXPONENTIAL:
            result = ExponentialFilter(stabilizer);
            break;
        case FILTER_KALMAN:
            result = KalmanFilter_Apply(stabilizer);
            break;
        default:
            result = ExponentialFilter(stabilizer);
            break;
    }
    
    return result;
}