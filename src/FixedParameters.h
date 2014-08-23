#pragma once

//namespace TrackerConstants {

/*
#define IMG_DIM	500.0 
#define FIXED_SCALE 0.9
#define FIXED_KERNEL 10
*//*
static const float IMG_DIM = 250.0;
static const float FIXED_SCALE = 0.45;
static const int FIXED_KERNEL = 6;
*/
/*
static const float IMG_DIM = 64.0;
static const float FIXED_SCALE = 0.16;
static const int FIXED_KERNEL = 2;
*/

#define IMG_DIM	150.0 
#define FIXED_SCALE 0.3 
#define FIXED_KERNEL 4  
/*
#define IMG_DIM	100.0 
#define FIXED_SCALE 0.2 
#define FIXED_KERNEL 2 

#define IMG_DIM	50.0
#define FIXED_SCALE 0.1 
#define FIXED_KERNEL 0
*/

#define	NUM_FINGERS	5

#define	TINY_IMG_DIM		150 // duplicate in handModel -> TODO: unite image sizes  (or not -> one size for processing, one for user)
#define FTIP_HIST_SIZE		3	// history array size

#define	HAND_GRABBED			"hand_grabbed"
#define HAND_RELEASED			"hand_released"
#define HAND_PICKED				"hand_picked"

#define MIN_HAND_DEPTH		 500.0
#define MAX_HAND_DEPTH	    2000.0

//}