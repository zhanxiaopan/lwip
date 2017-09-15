/**
 * kalman_filter.c
 *
 *  Created on: Nov 23, 2016
 *      Author: Wenlong Li
 * Description
 *  - this is to implement Kalman filter.
 *	- only the filter for scalar signal is implemented here.
 *	- for more details, check http://bilgin.esme.org/BitsAndBytes/KalmanFilterforDummies
 * History
 *	- v0.1 created. only support the simplest situation: A=1, B=1, H=1, u=0 in this version.
 */
 
#include "kalman_filter.h"

/**
 *  @brief kalman filter calculation
 *  
 *  @param [in] kf the kalman filter entity to process
 *  @return None
 *  
 *  @details for the simplest kalman filter format so far.
 */
void kalman_filter_process (KALMAN_FILTER_T *kf)
{
	/*Read new measurement data*/
	kf->Zk = *(kf->rawdata);
	
	/* Time update for prediction */
	kf->prior_X = kf->Xk;
	kf->prior_P = kf->Pk + kf->cov_Q;
	
	/* Measurement update for correction */
	kf->Kk = kf->prior_P / (kf->prior_P + kf->cov_R);
	kf->Xk = kf->prior_X + kf->Kk * (kf->Zk - kf->prior_X);
	kf->Pk = (1 - kf->Kk) * kf->prior_P;
}

/**
 *  @brief Init the kalman filter struct
 *  
 *  @param [in] kf   kalman filter
 *  @param [in] pRaw pointer to the src data to filter
 *  @param [in] covR R, covariance of measurement nosie
 *  @param [in] covQ Q, covariance of process nosie
 *  @return None
 *  
 *  @details suppose here:
 *  		  - X0 is 0
 *  		  - P0 is 1.
 */
void kalman_filter_init (KALMAN_FILTER_T *kf, double *pRaw, double covR, double covQ)
{
	/*  init X0 as 0
	 *	init P0 as 1
	 *	both has quite little influence on filtering result
	*/
	kf->Xk = 0;
	kf->Pk = 1;
	
	kf->cov_R = covR;
	kf->cov_Q = covQ;
	
	kf->rawdata = pRaw;	
}


#if 0
/* this is just an example to show how to utilize the kalman filter defined here */
double data_m; 			// this is the measured data
double cov_R_1 = 0.1;
double cov_Q_1 = 0.000001;
KALMAN_FILTER_T kf1;	// definition of kalman filter

kalman_filter_init (&kf1, &data_m, cov_R_1, cov_Q_1);

kalman_filter_process (&kf1);
#endif
