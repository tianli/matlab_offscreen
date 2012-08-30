camera {

        #declare rad = 15;         
        #declare angleA = array[27][2]{
	{0, 0},    // top view
	{90, 0}, // front view
	{90, 90},  // side view
	{90, -90}, // side view
	{90, 180},  // back view
	{40, 150},  // back side view 1
	{40, -150}, // back side view 2  
	{130, -180}, // back down view 3
	{110, -60}, // front down view 1
	{125, -30}, // front down view 2
	{110, 0}, // front down view 3
	{125, 30}, // front down view 4
	{110, 60}, // front down view 5		
	{60, -60}, // front level1 view
	{60, -30},
	{60, 0},
	{60, 30},
	{60, 60},
	{90, -60},  // front level2 view       #18
	{30, -36},
	{30, -12},
	{30, 12},
	{30, 36},
	{90, 60},
        {75, 45},            // new views 1
        {45, -45},           // new views 2
        {80, -150}       // new views 3		
	}; 
                            

        //location rad * <sin(angleA[clock][0]/180*pi)*sin(angleA[clock][1]/180*pi), cos(angleA[clock][0]/180*pi), sin(angleA[clock][0]/180*pi)*cos(angleA[clock][1]/180*pi)>
        
        #declare current = 24;
        location rad * <sin(angleA[current][0]/180*pi)*sin(angleA[current][1]/180*pi), cos(angleA[current][0]/180*pi), sin(angleA[current][0]/180*pi)*cos(angleA[current][1]/180*pi)>

	
	up < 0, 5, 0 >
	right < -6.66667, 0, 0 >
	direction < 0, 0, 9 >

	look_at < 0, 0, 0>

}