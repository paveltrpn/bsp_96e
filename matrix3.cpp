#include "global.h"

float InvQ[4][4];
int singular;

float angle(float x, float y)
{
    return -atan2(x,y);
    /*	if (fabs(x) < epsilon) {		//if x is zero
    		if (fabs(y) < epsilon) {	// y is zero
    			return (0.0);
    		} else if (y > 0.0) {			// y is positive
    			return (0.5*M_PI);
    		} else {					// y is negative
    			return (1.5*M_PI);
    		}
    	} else if (x < 0.0)	{			// x is negative
    		return (atan(y/x)+M_PI);
    	} else {						// x is positive
    		return (atan(y/x));
    	}*/
}

void tran3(float tx, float ty, float tz, float A[][4])
{
    for (int i=0; i<4; i++)
    {
        A[i][0] = 0.0f;
        A[i][1] = 0.0f;
        A[i][2] = 0.0f;
        A[i][3] = 0.0f;
        A[i][i] = 1.0f;
    }
    A[0][3] = -tx;
    A[1][3] = -ty;
    A[2][3] = -tz;
}

void scale3(float sx, float sy, float sz, float A[][4])
{
    for (int i=0; i<4; i++)
    {
        A[i][0] = 0.0f;
        A[i][1] = 0.0f;
        A[i][2] = 0.0f;
        A[i][3] = 0.0f;
    }
    A[0][0] = sx;
    A[1][1] = sy;
    A[2][2] = sz;
    A[3][3] = 1.0;
}

void rot3(int m, float th, float A[][4])
{
    int m1, m2;
    float c, s;

    for (int i=0; i<4; i++)
    {
        A[i][0] = 0.0f;
        A[i][1] = 0.0f;
        A[i][2] = 0.0f;
        A[i][3] = 0.0f;
    }

    A[m-1][m-1] = 1.0;
    A[3][3] = 1.0;
    m1 = (m % 3)+1;
    m2 = (m1 % 3)+1;
    c = cosl(th);
    s = sinl(th);

    A[m1-1][m1-1] = c;
    A[m2-1][m2-1] = c;
    A[m1-1][m2-1] = s;
    A[m2-1][m1-1] = -s;
}

void mult3(float A[][4], float B[][4], float C[][4])
{
    float ab;

    for (int i=0; i<4; i++)
    {
        for (int j=0; j<4; j++)
        {

            ab = A[i][0]*B[0][j] +
                 A[i][1]*B[1][j] +
                 A[i][2]*B[2][j] +
                 A[i][3]*B[3][j];

            C[i][j] = ab;
        }
    }
}

void rotate3(float phi, float *b, float *d, float A[4][4])
{
    float F[4][4], G[4][4], H[4][4], W[4][4];
    float FI[4][4], GI[4][4], HI[4][4], S[4][4], T[4][4];

    float beta, theta, v;

    tran3( b[0],  b[1],  b[2], F);
    tran3(-b[0], -b[1], -b[2], FI);

    theta = angle(d[0], d[1]);

    rot3(3, theta, G);
    rot3(3, -theta, GI);

    v = (float)sqrt(d[0]*d[0] + d[1]*d[1]);
    beta = angle(d[2], v);

    rot3(2, beta, H);
    rot3(2, -beta, HI);

    rot3(3, phi, W);

    mult3(G,F,S);
    mult3(H,S,T);
    mult3(W,T,S);
    mult3(HI,S,T);
    mult3(GI,T,S);
    mult3(FI,S,A);
}

void transform(float *v, float A[4][4], float *w)
{
    w[0] = A[0][0]*v[0] + A[0][1]*v[1] + A[0][2]*v[2] + A[0][3];
    w[1] = A[1][0]*v[0] + A[1][1]*v[1] + A[1][2]*v[2] + A[1][3];
    w[2] = A[2][0]*v[0] + A[2][1]*v[1] + A[2][2]*v[2] + A[2][3];
}

void transform2(float x, float y, float z, float A[4][4], float *w)
{
    w[0] = A[0][0]*x + A[0][1]*y + A[0][2]*z + A[0][3];
    w[1] = A[1][0]*x + A[1][1]*y + A[1][2]*z + A[1][3];
    w[2] = A[2][0]*x + A[2][1]*y + A[2][2]*z + A[2][3];
}

void findQRotate(float Q[4][4],float R[4][4],float *eye,float roll, float pitch, float yaw)
{
    float P[4][4];
    float Y[4][4];
    float T[4][4];
    float T1[4][4];

    rot3(3,roll,R);
    rot3(1,pitch,P);
    rot3(2,yaw,Y);

    tran3(eye[0],eye[1],eye[2],T);
    mult3(R,P,T1);
    mult3(Y,T1,R);
    mult3(R,T,Q);
}


/////////////////////////////////////////////////////////////////////
// Concatenate two 3x3 matrices.
/////////////////////////////////////////////////////////////////////
void MConcat(float in1[3][3], float in2[3][3], float out[3][3])
{
    for (int i=0 ; i<3 ; i++)
    {
        out[i][0] = in1[i][0] * in2[0][0] + in1[i][1] * in2[1][0] + in1[i][2] * in2[2][0];
        out[i][1] = in1[i][0] * in2[0][1] + in1[i][1] * in2[1][1] + in1[i][2] * in2[2][1];
        out[i][2] = in1[i][0] * in2[0][2] + in1[i][1] * in2[1][2] + in1[i][2] * in2[2][2];
    }
}

void findQ(float Q[4][4],float R[4][4],float *eye,float roll, float pitch, float yaw)
{
    //initializing here fixes a bug! also, static improves perf (tested only with editwin timerefresh)
    static float T2[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};

    float  mroll[3][3]  = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    float  mpitch[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    float  myaw[3][3]   = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    //vec3_t vpn, vright, vup;

    float  s, c, mtemp1[3][3], mtemp2[3][3];

    s = sin(pitch);
    c = cos(pitch);
    mroll[0][0] = c;
    mroll[0][1] = s;
    mroll[1][0] = -s;
    mroll[1][1] = c;

    s = sin(-yaw + 1.5*M_PI);
    c = cos(-yaw + 1.5*M_PI);
    mpitch[1][1] = c;
    mpitch[1][2] = s;
    mpitch[2][1] = -s;
    mpitch[2][2] = c;

    roll = 0;	//roll removed
    s = sin(roll + M_PI/2.0f);
    c = cos(roll + M_PI/2.0f);
    myaw[0][0] = c;
    myaw[0][2] = -s;
    myaw[2][0] = s;
    myaw[2][2] = c;

    MConcat(mpitch, myaw, mtemp1);      // myaw, mtemp1, mtemp2
    MConcat(mroll, mtemp1, mtemp2);
    float SC[4][4], T3[4][4];
    // rotate axis 1 by -90 and axis 3 by 90

    for (int i=0 ; i<3 ; i++)
    {
        T2[2][i] = mtemp2[0][i];
        T2[1][i] = mtemp2[1][i];
        T2[0][i] = mtemp2[2][i];
    }

    rot3(1,-M_PI/2.0f,T3);
    mult3(T3, T2, SC);
    rot3(3,M_PI/2.0f,T3);
    mult3(SC, T3, T2);

    scale3(1,1,1,SC);
    mult3(T2,SC,R);

    T2[0][3] = -eye[0];
    T2[1][3] = -eye[1];
    T2[2][3] = -eye[2];

    mult3(T2,SC,Q);

    //fov / zoom
    float fov = 1 / DEG2RAD(set.fov);
    Q[0][0] *= fov;
    Q[0][1] *= fov;
    Q[0][2] *= fov;
    Q[1][0] *= fov;
    Q[1][1] *= fov;
    Q[1][2] *= fov;
}

void findQ2(float Q[4][4],float *eye,float *lookAt)
{
    vec3_t direct;
    float a, b, c, v, w;
    float E[4][4];
    float F[4][4];
    float G[4][4];
    float H[4][4];
    float U[4][4];

    VectorSubtract(lookAt,eye,direct);

    tran3(eye[0],eye[1],eye[2],F);
    a = angle(-direct[0],-direct[1]);

    rot3(3,a,G);

    v = sqrt(direct[0]*direct[0]+direct[1]*direct[1]);

    b = angle(-direct[2],v);

    rot3(2,b,H);

    w = sqrt(v*v+direct[2]*direct[2]);

    c = angle(-direct[0]*w,direct[1]*direct[2]);

    rot3(3,-c,U);

    mult3(G,F,Q);
    mult3(H,Q,E);
    mult3(U,E,Q);
};

void findQ3(float Q[4][4],float *eye,float *lookAt)
{
    vec3_t direct;
    float a, b, c, v, w;
    float E[4][4];
    float F[4][4];
    float G[4][4];
    float H[4][4];
    float U[4][4];

    VectorSubtract(lookAt,eye,direct);

    tran3(eye[0],eye[1],eye[2],F);
    a = angle(-direct[0],-direct[2]);

    rot3(2,a,G);

    v = sqrt(direct[0]*direct[0]+direct[2]*direct[2]);

    b = angle(-direct[1],v);

    rot3(3,b,H);

    w = sqrt(v*v+direct[1]*direct[1]);

    c = angle(-direct[0]*w,direct[2]*direct[1]);

    rot3(2,-c,U);

    mult3(G,F,Q);
    mult3(H,Q,E);
    mult3(U,E,Q);
};

void invert(float A[4][4],float AInv[4][4],int *sing)
{
    float det, adj;

    int i, i1, i2, j, j1, j2;

    det = A[1][1]*(A[2][2]*A[3][3]-A[2][3]*A[3][2])+
          A[1][2]*(A[2][3]*A[3][1]-A[2][1]*A[3][3])+
          A[1][3]*(A[2][1]*A[3][2]-A[2][2]*A[3][1]);

    if (!det)
    {
        *sing = 1;
    }
    else
    {
        *sing = 0;
        for (i=1; i<4; i++)
        {
            i1 = (i % 3)+1;
            i2 = (i1 % 3)+1;
            for (j=1; j<4; j++)
            {
                j1 = (j % 3)+1;
                j2 = (j1 %3)+1;
                adj = (A[i1][j1]*A[i2][j2] - A[i1][j2]*A[i2][j1]);
                AInv[j][i] = adj/det;
            };
        };
    };
};

/* not used.

int ClipXY(int npts, TRect CR)
{
	int xmin, xmax, ymin, ymax;

	xmin = CR.left;
	xmax = CR.right;
	ymin = CR.top;
	ymax = CR.bottom;

	float m;
	float dx, dy;

	int cp = 0;
	int curpts;
	int v1 = npts - 1;
	int v2;

//	vec3_t **pt3;

//	pt3 = new vec3_t *[2];
	vec3_t *pt3[2];
	pt3[0] = new vec3_t[CPT];
	pt3[1] = new vec3_t[CPT];

	int i, j;

	curpts = npts;
	i = 0;
	j = 1;

	// first transfer the points to pt array...
	for (int counter = 0; counter < npts; counter++) {
		pt3[i][counter][0] = tpts[counter][0];
		pt3[i][counter][1] = tpts[counter][1];
		pt3[i][counter][2] = tpts[counter][2];
	};

	i ^= 1; // swap...
	j ^= 1;
// ************ Left Side ********************* //
	for (v2 =0; v2<curpts; v2++) {
		if ((pt3[j][v1][0] >= xmin*16) && (pt3[j][v2][0] >= xmin*16)) {
			pt3[i][cp][0]   = pt3[j][v2][0];
			pt3[i][cp][1]   = pt3[j][v2][1];
			pt3[i][cp][2]   = pt3[j][v2][2];
			cp++;
		}
		if ((pt3[j][v1][0] < xmin*16) && (pt3[j][v2][0] < xmin*16)) {
		}
		if ((pt3[j][v1][0] >= xmin*16) && (pt3[j][v2][0] < xmin*16)) {
			dy = (float)(pt3[j][v2][1] - pt3[j][v1][1]);
			dx = (float)(pt3[j][v2][0] - pt3[j][v1][0]);
			m = dy/dx;

			pt3[i][cp][0]   = xmin*16;
			pt3[i][cp][1]   = pt3[j][v1][1]+m*(xmin*16 - pt3[j][v1][0]);
			pt3[i][cp][2]   = pt3[j][v1][2];
			cp++;
		}
		if ((pt3[j][v1][0] < xmin*16) && (pt3[j][v2][0] >= xmin*16)) {
			dy = (float)(pt3[j][v2][1] - pt3[j][v1][1]);
			dx = (float)(pt3[j][v2][0] - pt3[j][v1][0]);
			m = dy/dx;

			pt3[i][cp][0]   = xmin*16;
			pt3[i][cp][1]   = pt3[j][v1][1]+m*(xmin*16 - pt3[j][v1][0]);
			pt3[i][cp][2]   = pt3[j][v1][2];
			cp++;
			pt3[i][cp][0]   = pt3[j][v2][0];
			pt3[i][cp][1]   = pt3[j][v2][1];
			pt3[i][cp][2]   = pt3[j][v2][2];
			cp++;
		}
		v1 = v2;
	};

	curpts = cp;
	cp = 0;
// ************ Right Side ********************* //
	v1 = curpts - 1;
	i ^= 1; // swap...
	j ^= 1;
	for (v2 =0; v2 < curpts; v2++) {
		if ((pt3[j][v1][0] <= xmax*16) && (pt3[j][v2][0] <= xmax*16)) {
			pt3[i][cp][0]   = pt3[j][v2][0];
			pt3[i][cp][1]   = pt3[j][v2][1];
			pt3[i][cp][2]   = pt3[j][v2][2];
			cp++;

		}
		if ((pt3[j][v1][0] > xmax*16) && (pt3[j][v2][0] > xmax*16)) {
		}
		if ((pt3[j][v1][0] <= xmax*16) && (pt3[j][v2][0] > xmax*16)) {
			dy = (float)(pt3[j][v2][1] - pt3[j][v1][1]);
			dx = (float)(pt3[j][v2][0] - pt3[j][v1][0]);
			m = dy/dx;

			pt3[i][cp][0]   = xmax*16;
			pt3[i][cp][1]   = pt3[j][v1][1]+m*(xmax*16 - pt3[j][v1][0]);
			pt3[i][cp][2]   = pt3[j][v1][2];
			cp++;
	}
		if ((pt3[j][v1][0] > xmax*16) && (pt3[j][v2][0] <= xmax*16)) {
			dy = (float)(pt3[j][v2][1] - pt3[j][v1][1]);
			dx = (float)(pt3[j][v2][0] - pt3[j][v1][0]);
			m = dy/dx;

			pt3[i][cp][0]   = xmax*16;
			pt3[i][cp][1]   = pt3[j][v1][1]+m*(xmax*16 - pt3[j][v1][0]);
			pt3[i][cp][2]   = pt3[j][v1][2];
			cp++;
			pt3[i][cp][0]   = pt3[j][v2][0];
			pt3[i][cp][1]   = pt3[j][v2][1];
			pt3[i][cp][2]   = pt3[j][v2][2];
			cp++;
		}
		v1 = v2;
	};

	curpts = cp;
	cp = 0;

// ************ Top Side ********************* //
	v1 = curpts - 1;
	i ^= 1; // swap...
	j ^= 1;
	for (v2 =0; v2< curpts; v2++) {
		if ((pt3[j][v1][1] >= ymin*16) && (pt3[j][v2][1] >= ymin*16)) {
			pt3[i][cp][0]   = pt3[j][v2][0];
			pt3[i][cp][1]   = pt3[j][v2][1];
			pt3[i][cp][2]   = pt3[j][v2][2];
			cp++;
		}
		if ((pt3[j][v1][1] < ymin*16) && (pt3[j][v2][1] < ymin*16)) {
		}
		if ((pt3[j][v1][1] >= ymin*16) && (pt3[j][v2][1] < ymin*16)) {
			dy = (float)(pt3[j][v2][1] - pt3[j][v1][1]);
			dx = (float)(pt3[j][v2][0] - pt3[j][v1][0]);

			if (dx) {
				m = dy/dx;
				pt3[i][cp][0] = pt3[j][v1][0]+(ymin*16 - pt3[j][v1][1])/m;
			} else {
				pt3[i][cp][0] = pt3[j][v1][0];
			};
			pt3[i][cp][1]    = ymin*16;
			pt3[i][cp][2]    = pt3[j][v1][2];
			cp++;
		}
		if ((pt3[j][v1][1] < ymin*16) && (pt3[j][v2][1] >= ymin*16)) {
			dy = (float)(pt3[j][v2][1] - pt3[j][v1][1]);
			dx = (float)(pt3[j][v2][0] - pt3[j][v1][0]);

			if (dx) {
				m = dy/dx;
				pt3[i][cp][0] = pt3[j][v1][0]+(ymin*16 - pt3[j][v1][1])/m;
			} else {
				pt3[i][cp][0] = pt3[j][v1][0];
			};
			pt3[i][cp][1]    = ymin*16;
			pt3[i][cp][2]    = pt3[j][v1][2];
			cp++;
			pt3[i][cp][0]    = pt3[j][v2][0];
			pt3[i][cp][1]    = pt3[j][v2][1];
			pt3[i][cp][2]    = pt3[j][v2][2];
			cp++;
		}
		v1 = v2;
	};

	curpts = cp;
	cp = 0;

// ************ Bottom Side ********************* //
	v1 = curpts - 1;
	i ^= 1; // swap...
	j ^= 1;
	for (v2 =0; v2< curpts; v2++) {
		if ((pt3[j][v1][1] <= ymax*16) && (pt3[j][v2][1] <= ymax*16)) {
			pt3[i][cp][0] = pt3[j][v2][0];
			pt3[i][cp][1] = pt3[j][v2][1];
			pt3[i][cp][2] = pt3[j][v2][2];
			cp++;
		}
		if ((pt3[j][v1][1] > ymax*16) && (pt3[j][v2][1] > ymax*16)) {
		}
		if ((pt3[j][v1][1] <= ymax*16) && (pt3[j][v2][1] > ymax*16)) {
			dy = (float)(pt3[j][v2][1] - pt3[j][v1][1]);
			dx = (float)(pt3[j][v2][0] - pt3[j][v1][0]);

			if (dx) {
				m = dy/dx;
				pt3[i][cp][0] = pt3[j][v1][0]+(ymax*16 - pt3[j][v1][1])/m;
			} else {
				pt3[i][cp][0] = pt3[j][v1][0];
			};
			pt3[i][cp][1]    = ymax*16;
			pt3[i][cp][2]    = pt3[j][v1][2];
			cp++;
		}
		if ((pt3[j][v1][1] > ymax*16) && (pt3[j][v2][1] <= ymax*16)) {
			dy = (float)(pt3[j][v2][1] - pt3[j][v1][1]);
			dx = (float)(pt3[j][v2][0] - pt3[j][v1][0]);

			if (dx) {
				m = dy/dx;
				pt3[i][cp][0] = pt3[j][v1][0]+(ymax*16 - pt3[j][v1][1])/m;
			} else {
				pt3[i][cp][0] = pt3[j][v1][0];
			};
			pt3[i][cp][1]    = ymax*16;
			pt3[i][cp][2]    = pt3[j][v1][2];
			cp++;
			pt3[i][cp][0]    = pt3[j][v2][0];
			pt3[i][cp][1]    = pt3[j][v2][1];
			pt3[i][cp][2]    = pt3[j][v2][2];
			cp++;
		}
		v1 = v2;
	};
	curpts = cp;

//	copy the points to the SPoints List.
	for (v1 = 0; v1<curpts; v1++) {
		SPoints[v1][0]    = pt3[i][v1][0];
		SPoints[v1][1]    = pt3[i][v1][1];
		SPoints[v1][2]    = pt3[i][v1][2];
	};
	delete[] pt3[0];
	delete[] pt3[1];
//	delete pt3;
	return (cp);
}
*/


int FastClipXY(int npts, LPRECT CR)
{
    //moved from bsp.cpp. was global and heap alloc'd before??
    static vec3_t pt[2][CPT];

    int xmin = CR->left;
    int xmax = CR->right;
    int ymin = CR->top;
    int ymax = CR->bottom;

    float dx, dy;

    int cp = 0;
    int v1 = npts - 1;
    int v2;

    int curpts = npts;
    int i = 0;
    int j = 1;


    // first transfer the points to pt array...
    /*	for (int counter = 0; counter < npts; counter++) {
    		pt[i][counter][0] = tpts[counter][0];
    		pt[i][counter][1] = tpts[counter][1];
    	};
    */
//	i ^= 1; // swap...
//	j ^= 1;

// ************ Left Side ********************* //
    for (v2 = 0; v2 < curpts; v2++)
    {
        if ((tpts[v1][0] >= xmin) && (tpts[v2][0] >= xmin))
        {
            pt[i][cp][0]   = tpts[v2][0];
            pt[i][cp][1]   = tpts[v2][1];
            cp++;
        }
        if ((tpts[v1][0] >= xmin) && (tpts[v2][0] < xmin))
        {
            dy = (tpts[v2][1] - tpts[v1][1]);
            dx = (tpts[v2][0] - tpts[v1][0]);

            pt[i][cp][0]   = xmin;
            pt[i][cp][1]   = tpts[v1][1]+dy*(xmin - tpts[v1][0])/dx;
            cp++;
        }
        if ((tpts[v1][0] < xmin) && (tpts[v2][0] >= xmin))
        {
            dy = (tpts[v2][1] - tpts[v1][1]);
            dx = (tpts[v2][0] - tpts[v1][0]);

            pt[i][cp][0]   = xmin;
            pt[i][cp][1]   = tpts[v1][1]+dy*(xmin - tpts[v1][0])/dx;
            cp++;
            pt[i][cp][0]   = tpts[v2][0];
            pt[i][cp][1]   = tpts[v2][1];
            cp++;
        }
        v1 = v2;
    };

    if (!cp) return 0;

    curpts = cp;
    cp = 0;

// ************ Top Side ********************* //
    v1 = curpts - 1;
    i ^= 1; // swap...
    j ^= 1;
    for (v2 =0; v2< curpts; v2++)
    {
        if ((pt[j][v1][1] >= ymin) && (pt[j][v2][1] >= ymin))
        {
            pt[i][cp][0]   = pt[j][v2][0];
            pt[i][cp][1]   = pt[j][v2][1];
            cp++;
        }
        if ((pt[j][v1][1] >= ymin) && (pt[j][v2][1] < ymin))
        {
            dy = (pt[j][v2][1] - pt[j][v1][1]);
            dx = (pt[j][v2][0] - pt[j][v1][0]);

            pt[i][cp][0] = pt[j][v1][0];
            if (dx)
            {
                pt[i][cp][0] += (ymin - pt[j][v1][1])*dx/dy;
            }
            pt[i][cp][1]    = ymin;
            cp++;
        }
        if ((pt[j][v1][1] < ymin) && (pt[j][v2][1] >= ymin))
        {
            dy = (pt[j][v2][1] - pt[j][v1][1]);
            dx = (pt[j][v2][0] - pt[j][v1][0]);

            pt[i][cp][0] = pt[j][v1][0];
            if (dx)
            {
                pt[i][cp][0] += (ymin - pt[j][v1][1])*dx/dy;
            }
            pt[i][cp][1]    = ymin;
            cp++;
            pt[i][cp][0]    = pt[j][v2][0];
            pt[i][cp][1]    = pt[j][v2][1];
            cp++;
        }
        v1 = v2;
    }

    if (!cp) return 0;

    curpts = cp;
    cp = 0;
// ************ Right Side ********************* //
    v1 = curpts - 1;
    i ^= 1; // swap...
    j ^= 1;
    for (v2 =0; v2 < curpts; v2++)
    {
        if ((pt[j][v1][0] <= xmax) && (pt[j][v2][0] <= xmax))
        {
            pt[i][cp][0]   = pt[j][v2][0];
            pt[i][cp][1]   = pt[j][v2][1];
            cp++;
        }
        if ((pt[j][v1][0] <= xmax) && (pt[j][v2][0] > xmax))
        {
            dy = (pt[j][v2][1] - pt[j][v1][1]);
            dx = (pt[j][v2][0] - pt[j][v1][0]);

            pt[i][cp][0]   = xmax;
            pt[i][cp][1]   = pt[j][v1][1]+dy*(xmax - pt[j][v1][0])/dx;
            cp++;
        }
        if ((pt[j][v1][0] > xmax) && (pt[j][v2][0] <= xmax))
        {
            dy = (pt[j][v2][1] - pt[j][v1][1]);
            dx = (pt[j][v2][0] - pt[j][v1][0]);

            pt[i][cp][0]   = xmax;
            pt[i][cp][1]   = pt[j][v1][1]+dy*(xmax - pt[j][v1][0])/dx;
            cp++;
            pt[i][cp][0]   = pt[j][v2][0];
            pt[i][cp][1]   = pt[j][v2][1];
            cp++;
        }
        v1 = v2;
    };

    if (!cp) return 0;

    curpts = cp;
    cp = 0;

// ************ Bottom Side ********************* //
    v1 = curpts - 1;
    j ^= 1;
    for (v2 =0; v2< curpts; v2++)
    {
        if ((pt[j][v1][1] <= ymax) && (pt[j][v2][1] <= ymax))
        {
            FSPoints[cp].x = (int)pt[j][v2][0];
            FSPoints[cp].y = (int)pt[j][v2][1];
            cp++;
        }
        if ((pt[j][v1][1] <= ymax) && (pt[j][v2][1] > ymax))
        {
            dy = (pt[j][v2][1] - pt[j][v1][1]);
            dx = (pt[j][v2][0] - pt[j][v1][0]);

            FSPoints[cp].x = (int)(pt[j][v1][0]);
            if (dx)
            {
                FSPoints[cp].x += (int)((ymax - pt[j][v1][1])*dx/dy);
            }
            FSPoints[cp].y    = (int)ymax;
            cp++;
        }
        if ((pt[j][v1][1] > ymax) && (pt[j][v2][1] <= ymax))
        {
            dy = (pt[j][v2][1] - pt[j][v1][1]);
            dx = (pt[j][v2][0] - pt[j][v1][0]);

            FSPoints[cp].x = (int)(pt[j][v1][0]);
            if (dx)
            {
                FSPoints[cp].x += (int)((ymax - pt[j][v1][1])*dx/dy);
            }

            FSPoints[cp].y    = (int)ymax;
            cp++;
            FSPoints[cp].x    = (int)pt[j][v2][0];
            FSPoints[cp].y    = (int)pt[j][v2][1];
            cp++;
        }
        v1 = v2;
    }
    return cp;
}
/*
int FastClipRen(int npts, int rw, int rh)
{
	int xmin = 0;
	int xmax = rw - 1;
	int ymin = 0;
	int ymax = rh - 1;

	float dx, dy;

	int cp = 0;
	int v1 = npts - 1;
	int v2;

	int curpts = npts;
	int i = 0;
	int j = 1;

	// first transfer the points to pt array...
//	for (int counter = 0; counter < npts; counter++) {
//		pt[i][counter][0] = tpts[counter][0];
//		pt[i][counter][1] = tpts[counter][1];
//	};

//	i ^= 1; // swap...
//	j ^= 1;
// ************ Left Side ********************* //
	for (v2 = 0; v2 < curpts; v2++) {
		if ((tpts[v1][0] >= xmin) && (tpts[v2][0] >= xmin)) {
			pt[i][cp][0]   = tpts[v2][0];
			pt[i][cp][1]   = tpts[v2][1];
			cp++;
		}
		if ((tpts[v1][0] >= xmin) && (tpts[v2][0] < xmin)) {
			dy = (tpts[v2][1] - tpts[v1][1]);
			dx = (tpts[v2][0] - tpts[v1][0]);

			pt[i][cp][0]   = xmin;
			pt[i][cp][1]   = tpts[v1][1]+dy*(xmin - tpts[v1][0])/dx;
			cp++;
		}
		if ((tpts[v1][0] < xmin) && (tpts[v2][0] >= xmin)) {
			dy = (tpts[v2][1] - tpts[v1][1]);
			dx = (tpts[v2][0] - tpts[v1][0]);

			pt[i][cp][0]   = xmin;
			pt[i][cp][1]   = tpts[v1][1]+dy*(xmin - tpts[v1][0])/dx;
			cp++;
			pt[i][cp][0]   = tpts[v2][0];
			pt[i][cp][1]   = tpts[v2][1];
			cp++;
		}
		v1 = v2;
	};

	if (!cp) return 0;

	curpts = cp;
	cp = 0;

// ************ Top Side ********************* //
	v1 = curpts - 1;
	i ^= 1; // swap...
	j ^= 1;
	for (v2 =0; v2< curpts; v2++) {
		if ((pt[j][v1][1] >= ymin) && (pt[j][v2][1] >= ymin)) {
			pt[i][cp][0]   = pt[j][v2][0];
			pt[i][cp][1]   = pt[j][v2][1];
			cp++;
		}
		if ((pt[j][v1][1] >= ymin) && (pt[j][v2][1] < ymin)) {
			dy = (pt[j][v2][1] - pt[j][v1][1]);
			dx = (pt[j][v2][0] - pt[j][v1][0]);

			if (dx) {
				pt[i][cp][0] = pt[j][v1][0]+(ymin - pt[j][v1][1])*dx/dy;
			} else {
				pt[i][cp][0] = pt[j][v1][0];
			};
			pt[i][cp][1]    = ymin;
			cp++;
		}
		if ((pt[j][v1][1] < ymin) && (pt[j][v2][1] >= ymin)) {
			dy = (pt[j][v2][1] - pt[j][v1][1]);
			dx = (pt[j][v2][0] - pt[j][v1][0]);

			if (dx) {
				pt[i][cp][0] = pt[j][v1][0]+(ymin - pt[j][v1][1])*dx/dy;
			} else {
				pt[i][cp][0] = pt[j][v1][0];
			};
			pt[i][cp][1]    = ymin;
			cp++;
			pt[i][cp][0]    = pt[j][v2][0];
			pt[i][cp][1]    = pt[j][v2][1];
			cp++;
		}
		v1 = v2;
	};

	if (!cp) return 0;

	curpts = cp;
	cp = 0;
// ************ Right Side ********************* //
	v1 = curpts - 1;
	i ^= 1; // swap...
	j ^= 1;
	for (v2 =0; v2 < curpts; v2++) {
		if ((pt[j][v1][0] <= xmax) && (pt[j][v2][0] <= xmax)) {
			pt[i][cp][0]   = pt[j][v2][0];
			pt[i][cp][1]   = pt[j][v2][1];
			cp++;
		}
		if ((pt[j][v1][0] <= xmax) && (pt[j][v2][0] > xmax)) {
			dy = (pt[j][v2][1] - pt[j][v1][1]);
			dx = (pt[j][v2][0] - pt[j][v1][0]);

			pt[i][cp][0]   = xmax;
			pt[i][cp][1]   = pt[j][v1][1]+dy*(xmax - pt[j][v1][0])/dx;
			cp++;
		}
		if ((pt[j][v1][0] > xmax) && (pt[j][v2][0] <= xmax)) {
			dy = (pt[j][v2][1] - pt[j][v1][1]);
			dx = (pt[j][v2][0] - pt[j][v1][0]);

			pt[i][cp][0]   = xmax;
			pt[i][cp][1]   = pt[j][v1][1]+dy*(xmax - pt[j][v1][0])/dx;
			cp++;
			pt[i][cp][0]   = pt[j][v2][0];
			pt[i][cp][1]   = pt[j][v2][1];
			cp++;
		}
		v1 = v2;
	};

	if (!cp) return 0;

	curpts = cp;
	cp = 0;

// ************ Bottom Side ********************* //
	v1 = curpts - 1;
	j ^= 1;
	for (v2 =0; v2< curpts; v2++) {
		if ((pt[j][v1][1] <= ymax) && (pt[j][v2][1] <= ymax)) {
			FSPoints[cp].x = (int)pt[j][v2][0];
			FSPoints[cp].y = (int)pt[j][v2][1];
			cp++;
		}
		if ((pt[j][v1][1] <= ymax) && (pt[j][v2][1] > ymax)) {
			dy = (pt[j][v2][1] - pt[j][v1][1]);
			dx = (pt[j][v2][0] - pt[j][v1][0]);

			if (dx) {
				FSPoints[cp].x = (int)(pt[j][v1][0]+(ymax - pt[j][v1][1])*dx/dy);
			} else {
				FSPoints[cp].x = (int)(pt[j][v1][0]);
			};
			FSPoints[cp].y    = (int)ymax;
			cp++;
		}
		if ((pt[j][v1][1] > ymax) && (pt[j][v2][1] <= ymax)) {
			dy = (pt[j][v2][1] - pt[j][v1][1]);
			dx = (pt[j][v2][0] - pt[j][v1][0]);

			if (dx) {
				FSPoints[cp].x = (int)(pt[j][v1][0]+(ymax - pt[j][v1][1])*dx/dy);
			} else {
				FSPoints[cp].x = (int)(pt[j][v1][0]);
			};
			FSPoints[cp].y    = (int)ymax;
			cp++;
			FSPoints[cp].x    = (int)pt[j][v2][0];
			FSPoints[cp].y    = (int)pt[j][v2][1];
			cp++;
		}
		v1 = v2;
	};
	return cp;
}
*/

// do near zmin and far zmax clipping...
int Z_Clip(int npts, float zmin, float zmax)
{
    int cp = 0;
    int npts1;
    int v1 = npts-1;
    int v2;
    //vec3_t p1, p2;
    //vec3_t midp
    vec3_t pv1, pv2;
    float t;

    for (v2 = 0; v2 < npts; v2++)
    {
        VectorCopy(CP[Bsp::cur][v1],pv1);
        VectorCopy(CP[Bsp::cur][v2],pv2);

        if ((pv1[2] <= zmin) && (pv2[2] <= zmin))
        {
            VectorCopy(pv2,CP[Bsp::lst][cp++]);
        };

        if ((pv1[2] <= zmin) && (pv2[2] > zmin))
        {
            t = (zmin-pv1[2])/(pv2[2]-pv1[2]);
            CP[Bsp::lst][cp][0]   = pv1[0] + t*(pv2[0]-pv1[0]);
            CP[Bsp::lst][cp][1]   = pv1[1] + t*(pv2[1]-pv1[1]);
            CP[Bsp::lst][cp][2]   = zmin;
            cp++;
        };
        if ((pv1[2] > zmin) && (pv2[2] <= zmin))
        {
            t = (zmin-pv1[2])/(pv2[2]-pv1[2]);
            CP[Bsp::lst][cp][0]   = pv1[0] + t*(pv2[0]-pv1[0]);
            CP[Bsp::lst][cp][1]   = pv1[1] + t*(pv2[1]-pv1[1]);
            CP[Bsp::lst][cp][2]   = zmin;

            cp++;
            CP[Bsp::lst][cp][0]   = pv2[0];
            CP[Bsp::lst][cp][1]   = pv2[1];
            CP[Bsp::lst][cp][2]   = pv2[2];
            cp++;
        };
        v1 = v2;
    }

    if (cp <= 0)   // still some left;;;
    {
        return 0;
    }

    Bsp::cur ^= 1;
    Bsp::lst ^= 1;

    v1 = cp -1;
    npts1 = cp;
    cp = 0;

    for (v2 = 0; v2 < npts1; v2++)
    {
        VectorCopy(CP[Bsp::cur][v1],pv1);
        VectorCopy(CP[Bsp::cur][v2],pv2);

        if ((pv1[2] >= zmax) && (pv2[2] >= zmax))
        {
            VectorCopy(pv2,CP[Bsp::lst][cp++]);
        }

        if ((pv1[2] >= zmax) && (pv2[2] < zmax))
        {
            t = (zmax-pv1[2])/(pv2[2]-pv1[2]);
            CP[Bsp::lst][cp][0]   = pv1[0] + t*(pv2[0]-pv1[0]);
            CP[Bsp::lst][cp][1]   = pv1[1] + t*(pv2[1]-pv1[1]);
            CP[Bsp::lst][cp][2]   = zmax;
            cp++;
        };
        if ((pv1[2] < zmax) && (pv2[2] >= zmax))
        {
            t = (zmax-pv1[2])/(pv2[2]-pv1[2]);
            CP[Bsp::lst][cp][0]   = pv1[0] + t*(pv2[0]-pv1[0]);
            CP[Bsp::lst][cp][1]   = pv1[1] + t*(pv2[1]-pv1[1]);
            CP[Bsp::lst][cp][2]   = zmax;
            cp++;
            CP[Bsp::lst][cp][0]   = pv2[0];
            CP[Bsp::lst][cp][1]   = pv2[1];
            CP[Bsp::lst][cp][2]   = pv2[2];
            cp++;
        };
        v1 = v2;
    };
    return (cp);
}
/*
int XY_Clip(int npts, vec3_t *TP, TRect CR) {
	float m;
	float dx, dy;

	int xmin, xmax, ymin, ymax;

	xmin = CR.left;
	xmax = CR.right;
	ymin = CR.top;
	ymax = CR.bottom;

	int cp = 0;
	int curpts;
	int v1 = npts - 1;
	int v2;

	vec3_t **pt3;

	pt3 = new vec3_t *[2];
	pt3[0] = new vec3_t[CPT];
	pt3[1] = new vec3_t[CPT];

	int i, j;

	curpts = npts;
	i = 0;
	j = 1;

	// first transfer the points to pt array...
	for (int counter = 0; counter < npts; counter++) {
   	VectorCopy(TP[counter],pt3[i][counter]);
	};

	i ^= 1; // swap...
	j ^= 1;
// ************ Left Side ********************* //
	for (v2 =0; v2<curpts; v2++) {
		if ((pt3[j][v1][0] >= xmin*16) && (pt3[j][v2][0] >= xmin*16)) {
			pt3[i][cp][0]   = pt3[j][v2][0];
			pt3[i][cp][1]   = pt3[j][v2][1];
			pt3[i][cp][2]   = pt3[j][v2][2];
			cp++;
		}
	//	if ((pt3[j][v1][0] < xmin*16) && (pt3[j][v2][0] < xmin*16)) {
	//	}
		if ((pt3[j][v1][0] >= xmin*16) && (pt3[j][v2][0] < xmin*16)) {
			dy = (float)(pt3[j][v2][1] - pt3[j][v1][1]);
			dx = (float)(pt3[j][v2][0] - pt3[j][v1][0]);
			m = dy/dx;

			pt3[i][cp][0]   = xmin*16;
			pt3[i][cp][1]   = pt3[j][v1][1]+m*(xmin*16 - pt3[j][v1][0]);
			pt3[i][cp][2]   = pt3[j][v1][2];
			cp++;
		}
		if ((pt3[j][v1][0] < xmin*16) && (pt3[j][v2][0] >= xmin*16)) {
			dy = (float)(pt3[j][v2][1] - pt3[j][v1][1]);
			dx = (float)(pt3[j][v2][0] - pt3[j][v1][0]);
			m = dy/dx;

			pt3[i][cp][0]   = xmin*16;
			pt3[i][cp][1]   = pt3[j][v1][1]+m*(xmin*16 - pt3[j][v1][0]);
			pt3[i][cp][2]   = pt3[j][v1][2];
			cp++;
			pt3[i][cp][0]   = pt3[j][v2][0];
			pt3[i][cp][1]   = pt3[j][v2][1];
			pt3[i][cp][2]   = pt3[j][v2][2];
			cp++;
		}
		v1 = v2;
	};

	curpts = cp;
	cp = 0;
// ************ Right Side ********************* //
	v1 = curpts - 1;
	i ^= 1; // swap...
	j ^= 1;
	for (v2 =0; v2 < curpts; v2++) {
		if ((pt3[j][v1][0] <= xmax*16) && (pt3[j][v2][0] <= xmax*16)) {
			pt3[i][cp][0]   = pt3[j][v2][0];
			pt3[i][cp][1]   = pt3[j][v2][1];
			pt3[i][cp][2]   = pt3[j][v2][2];
			cp++;

		}
		//if ((pt3[j][v1][0] > xmax*16) && (pt3[j][v2][0] > xmax*16)) {
		//}
		if ((pt3[j][v1][0] <= xmax*16) && (pt3[j][v2][0] > xmax*16)) {
			dy = (float)(pt3[j][v2][1] - pt3[j][v1][1]);
			dx = (float)(pt3[j][v2][0] - pt3[j][v1][0]);
			m = dy/dx;

			pt3[i][cp][0]   = xmax*16;
			pt3[i][cp][1]   = pt3[j][v1][1]+m*(xmax*16 - pt3[j][v1][0]);
			pt3[i][cp][2]   = pt3[j][v1][2];
			cp++;
	}
		if ((pt3[j][v1][0] > xmax*16) && (pt3[j][v2][0] <= xmax*16)) {
			dy = (float)(pt3[j][v2][1] - pt3[j][v1][1]);
			dx = (float)(pt3[j][v2][0] - pt3[j][v1][0]);
			m = dy/dx;

			pt3[i][cp][0]   = xmax*16;
			pt3[i][cp][1]   = pt3[j][v1][1]+m*(xmax*16 - pt3[j][v1][0]);
			pt3[i][cp][2]   = pt3[j][v1][2];
			cp++;
			pt3[i][cp][0]   = pt3[j][v2][0];
			pt3[i][cp][1]   = pt3[j][v2][1];
			pt3[i][cp][2]   = pt3[j][v2][2];
			cp++;
		}
		v1 = v2;
	};

	curpts = cp;
	cp = 0;

// ************ Top Side ********************* //
	v1 = curpts - 1;
	i ^= 1; // swap...
	j ^= 1;
	for (v2 =0; v2< curpts; v2++) {
		if ((pt3[j][v1][1] >= ymin*16) && (pt3[j][v2][1] >= ymin*16)) {
			pt3[i][cp][0]   = pt3[j][v2][0];
			pt3[i][cp][1]   = pt3[j][v2][1];
			pt3[i][cp][2]   = pt3[j][v2][2];
			cp++;
		}
	//	if ((pt3[j][v1][1] < ymin*16) && (pt3[j][v2][1] < ymin*16)) {
	//	}
		if ((pt3[j][v1][1] >= ymin*16) && (pt3[j][v2][1] < ymin*16)) {
			dy = (float)(pt3[j][v2][1] - pt3[j][v1][1]);
			dx = (float)(pt3[j][v2][0] - pt3[j][v1][0]);

			if (dx) {
				m = dy/dx;
				pt3[i][cp][0] = pt3[j][v1][0]+(ymin*16 - pt3[j][v1][1])/m;
			} else {
				pt3[i][cp][0] = pt3[j][v1][0];
			};
			pt3[i][cp][1]    = ymin*16;
			pt3[i][cp][2]    = pt3[j][v1][2];
			cp++;
		}
		if ((pt3[j][v1][1] < ymin*16) && (pt3[j][v2][1] >= ymin*16)) {
			dy = (float)(pt3[j][v2][1] - pt3[j][v1][1]);
			dx = (float)(pt3[j][v2][0] - pt3[j][v1][0]);

			if (dx) {
				m = dy/dx;
				pt3[i][cp][0] = pt3[j][v1][0]+(ymin*16 - pt3[j][v1][1])/m;
			} else {
				pt3[i][cp][0] = pt3[j][v1][0];
			};
			pt3[i][cp][1]    = ymin*16;
			pt3[i][cp][2]    = pt3[j][v1][2];
			cp++;
			pt3[i][cp][0]    = pt3[j][v2][0];
			pt3[i][cp][1]    = pt3[j][v2][1];
			pt3[i][cp][2]    = pt3[j][v2][2];
			cp++;
		}
		v1 = v2;
	};

	curpts = cp;
	cp = 0;

// ************ Bottom Side ********************* //
	v1 = curpts - 1;
	i ^= 1; // swap...
	j ^= 1;
	for (v2 =0; v2< curpts; v2++) {
		if ((pt3[j][v1][1] <= ymax*16) && (pt3[j][v2][1] <= ymax*16)) {
			pt3[i][cp][0]   = pt3[j][v2][0];
			pt3[i][cp][1] = pt3[j][v2][1];
			pt3[i][cp][2] = pt3[j][v2][2];
			cp++;
		}
	//	if ((pt3[j][v1][1] > ymax*16) && (pt3[j][v2][1] > ymax*16)) {
	//	}
		if ((pt3[j][v1][1] <= ymax*16) && (pt3[j][v2][1] > ymax*16)) {
			dy = (float)(pt3[j][v2][1] - pt3[j][v1][1]);
			dx = (float)(pt3[j][v2][0] - pt3[j][v1][0]);

			if (dx) {
				m = dy/dx;
				pt3[i][cp][0] = pt3[j][v1][0]+(ymax*16 - pt3[j][v1][1])/m;
			} else {
				pt3[i][cp][0] = pt3[j][v1][0];
			};
			pt3[i][cp][1]    = ymax*16;
			pt3[i][cp][2]    = pt3[j][v1][2];
			cp++;
		}
		if ((pt3[j][v1][1] > ymax*16) && (pt3[j][v2][1] <= ymax*16)) {
			dy = (float)(pt3[j][v2][1] - pt3[j][v1][1]);
			dx = (float)(pt3[j][v2][0] - pt3[j][v1][0]);

			if (dx) {
				m = dy/dx;
				pt3[i][cp][0] = pt3[j][v1][0]+(ymax*16 - pt3[j][v1][1])/m;
			} else {
				pt3[i][cp][0] = pt3[j][v1][0];
			};
			pt3[i][cp][1]    = ymax*16;
			pt3[i][cp][2]    = pt3[j][v1][2];
			cp++;
			pt3[i][cp][0]    = pt3[j][v2][0];
			pt3[i][cp][1]    = pt3[j][v2][1];
			pt3[i][cp][2]    = pt3[j][v2][2];
			cp++;
		}
		v1 = v2;
	};
	curpts = cp;

	//	copy the points to the SPoints List.
	for (v1 = 0; v1<curpts; v1++) {
		SPoints[v1][0]    = pt3[i][v1][0];
		SPoints[v1][1]    = pt3[i][v1][1];
		SPoints[v1][2]    = pt3[i][v1][2];
	};

	delete[] pt3[0];
	delete[] pt3[1];
	delete pt3;
	return (cp);
}

int ClipZPoint(const float *pt, float zmin, float zmax)
{
	if (pt[2] > zmin || pt[2] < zmax)
		return 0;
	return 1;
}

int ClipXYPoint(const float *pt, LPRECT CR)
{
	if ((pt[0] < CR->left) || (pt[0] > CR->right) || (pt[1] < CR->top)  || (pt[1] > CR->bottom))
		return 0;
	return 1;
}
*/
