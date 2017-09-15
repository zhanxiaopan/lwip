#ifndef __USE_SCALAR_KALMAN_FILTER_H
#define __USE_SCALAR_KALMAN_FILTER_H

#include <stdint.h>


typedef struct KALMAN_FILTER_T {
	/* Kalman filter paramters */
	double cov_R;	// the covariance of measurement noise.
	double cov_Q;	// the covariance of process noise.
	
	/* Initial values */
	double P0;
	double X0;
	double K0;
	
	/* intermediate variable */
	double prior_P;
	double prior_X;
	
	/* src data to filter*/
	double *rawdata;
	
	/* Iterated var in this Momment*/
	double Kk;	// current estimation of K (Kalman gain)
	double Pk;	// current estimation of P
	double Zk;	// current measurement result.
	double Xk;	// current estiamted result (also the result of kalman filter)
} KALMAN_FILTER_T;

void kalman_filter_process(KALMAN_FILTER_T *kf);
void kalman_filter_init (KALMAN_FILTER_T *kf, double *pRaw, double covR, double covQ);

#endif
