

// bastardized from the GLFW particles example
// GLFW - http://glfw.sourceforge.net/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <inttypes.h>

#include "mylcd.h"
#include "demos.h"



#ifndef DEGTORAD
#define DEGTORAD 0.034906585039
#endif


// Life span of a particle (in seconds)

int destx;
int desty;
float xpos, ypos, zpos,angle_x, angle_y, angle_z;

const float zoom = -60.0; // camz
const float flength = 100;

// Maximum number of particles
#define MAX_PARTICLES   2000

// Life span of a particle (in seconds)
#define LIFE_SPAN 		16.0f

// A new particle is born every [BIRTH_INTERVAL] second
#define BIRTH_INTERVAL (LIFE_SPAN/(float)MAX_PARTICLES)

// Particle size (meters)
#define PARTICLE_SIZE   5.0f

// Gravitational constant (m/s^2)
#define GRAVITY         9.8f

// Base initial velocity (m/s)
#define VELOCITY        12.0f

// Bounce friction (1.0 = no friction, 0.0 = maximum friction)
#define FRICTION        0.75f

// "Fountain" height (m)
#define FOUNTAIN_HEIGHT 20.0f

// Fountain radius (m)
#define FOUNTAIN_RADIUS 1.6f

// Minimum delta-time for particle phisics (s)
#define MIN_DELTA_T     (BIRTH_INTERVAL*0.5)

typedef struct {
	float z;
	int x;
	int y;
	int c;
	int active;
}TZPOINT;

static TZPOINT points[MAX_PARTICLES];

// This structure holds all state for a single particle
typedef struct {
    float x,y,z;     // Position in space
    float vx,vy,vz;  // Velocity vector
    float r,g,b;     // Colour of particle
    float life;      // Life of particle (1.0 = newborn, < 0.0 = dead)
    int   active;    // Tells if this particle is active
} PARTICLE;

// Global vectors holding all particles. We use two vectors for float
// buffering.
static PARTICLE particles[ MAX_PARTICLES];

// Global variable holding the age of the youngest particle
static float min_age = 0.0f;


void DrawParticles(TFRAME *frame, float t, float dt);



static void vertToPoint (TFRAME *frame, float xx, float yy, float zz, int *x1, int *y1)
{
	const float ThetaX = /*xpos*/-53.0 * DEGTORAD; //xpos * DEGTORAD;
	const float ThetaY = ypos * DEGTORAD;
	const float ThetaZ = zpos * DEGTORAD;
		
//	float xx = x;
//	float yy = y;
	//float zz = z;
	lRotateX(ThetaX, yy, zz, &yy, &zz);
	lRotateY(ThetaY, xx, zz, &xx, &zz);
	lRotateZ(ThetaZ, xx, yy, &xx, &yy);

	lPoint3DTo2D(xx, yy, zz, flength, zoom, destx, desty, x1, y1);
	*y1 = (frame->height-1) - *y1;
//	*x1 = (frame->width-1) - *x1;
}

static INLINE int checkbounds (const TFRAME *const frm, const int x, const int y)
{
	if (x<0 || x>=frm->width || y>=frm->height || y<0)
		return 1;
	else
		return 0;
}


//========================================================================
// TesselateFloor() - Recursive function for building variable tesselated
// floor
//========================================================================

void TesselateFloor (TFRAME *frame, float x1, float y1, float x2, float y2, int recursion )
{
    float delta, x, y;
	
    // Last recursion?
    if( recursion >= 4 )
    {
        delta = 999999.0f;
    }
    else
    {
        x = fabs(x1) < fabs(x2) ? fabs(x1) : fabs(x2);
        y = fabs(y1) < fabs(y2) ? fabs(y1) : fabs(y2);
        delta = x*x + y*y;
    }

    // Recurse further?
    if( delta < 0.5f )
    {
        x = (x1+x2) * 0.5f;
        y = (y1+y2) * 0.5f;
        TesselateFloor(frame, x1,y1,  x, y, recursion + 1 );
        TesselateFloor(frame,  x,y1, x2, y, recursion + 1 );
        TesselateFloor(frame, x1, y,  x,y2, recursion + 1 );
        TesselateFloor(frame,  x, y, x2,y2, recursion + 1 );
    }
    else
    {

		int xx1, yy1;
		int xx2, yy2;
		vertToPoint(frame, x1*80.0f, y1*80.0f, 0.0f, &xx1, &yy1);
		vertToPoint(frame, x2*80.0f, y1*80.0f, 0.0f, &xx2, &yy2);

		if (!checkbounds(frame, xx1, yy1)){
			if (!checkbounds(frame, xx2, yy2))
				lDrawLine(frame, xx1, yy1, xx2, yy2, 0x00ffff);
		}

		vertToPoint(frame, x1*80.0f, y1*80.0f, 0.0f, &xx1, &yy1);
		vertToPoint(frame, x2*80.0f, y2*80.0f, 0.0f, &xx2, &yy2);
		
		if (!checkbounds(frame, xx1, yy1)){
			if (!checkbounds(frame, xx2, yy2))
				lDrawLine(frame, xx1, yy1, xx2, yy2, 0xff00ff);
		}
   	
    	/*
        glVertex3f( x1*80.0f, y1*80.0f , 0.0f );
        glVertex3f( x2*80.0f, y1*80.0f , 0.0f );
        glVertex3f( x2*80.0f, y2*80.0f , 0.0f );
        glVertex3f( x1*80.0f, y2*80.0f , 0.0f );
        */
    }
}





//========================================================================
// DrawFloor() - Draw floor. We builde the floor recursively, and let the
// tesselation in the centre (near x,y=0,0) be high, while the selleation
// around the edges be low.
//========================================================================

void DrawFloor (TFRAME *frame)
{
 /*
    static GLuint floor_list = 0;

    // Select floor texture
    if( !wireframe )
    {
        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, floor_tex_id );
    }

    // The first time, we build the floor display list
    if( !floor_list )
    {
        // Start recording of a new display list
        floor_list = glGenLists( 1 );
        glNewList( floor_list, GL_COMPILE_AND_EXECUTE );

        // Set floor material
        glMaterialfv( GL_FRONT, GL_DIFFUSE, floor_diffuse );
        glMaterialfv( GL_FRONT, GL_SPECULAR, floor_specular );
        glMaterialf(  GL_FRONT, GL_SHININESS, floor_shininess );

        // Draw floor as a bunch of triangle strips (high tesselation
        // improves lighting)
        glNormal3f( 0.0f, 0.0f, 1.0f );
        glBegin( GL_QUADS );
       */
        TesselateFloor(frame, -1.0f,-1.0f, 0.0f,0.0f, 0 );
        TesselateFloor(frame,  0.0f,-1.0f, 1.0f,0.0f, 0 );
        TesselateFloor(frame,  0.0f, 0.0f, 1.0f,1.0f, 0 );
        TesselateFloor(frame, -1.0f, 0.0f, 0.0f,1.0f, 0 );
        /*
        glEnd();

        // End recording of display list
        glEndList();
    }
    else
    {
        // Playback display list
        glCallList( floor_list );
    }

    glDisable( GL_TEXTURE_2D );
    */

}

void InitParticle (PARTICLE *p, float t)
{
    float xy_angle, velocity;

    // Start position of particle is at the fountain blow-out
    p->x = 0.0f;
    p->y = 0.0f;
    p->z = FOUNTAIN_HEIGHT;

    // Start velocity is up (Z)...
    p->vz = 0.7f + (0.3/4096.0) * (float) (rand() & 4095);

    // ...and a randomly chosen X/Y direction
    xy_angle = (2.0*M_PI/4096.0) * (float) (rand() & 4095);
    p->vx = 0.4f * (float) cosf( xy_angle );
    p->vy = 0.4f * (float) sinf( xy_angle );

    // Scale velocity vector according to a time-varying velocity
    velocity = VELOCITY*(0.8f + 0.1f*(float)(sin( 0.5*t )+sin( 1.31*t )));
    p->vx *= velocity;
    p->vy *= velocity;
    p->vz *= velocity;


    // Color is time-varying
    p->r = 0.7f + 0.3f * (float) sin( 0.34*t + 0.1 );
    p->g = 0.6f + 0.4f * (float) sin( 0.63*t + 1.1 );
    p->b = 0.6f + 0.4f * (float) sin( 0.91*t + 2.1 );

    // The particle is new-born and active
    p->life = 1.0f;
    p->active = 1;
}

#define FOUNTAIN_R2 (FOUNTAIN_RADIUS+PARTICLE_SIZE/2)*(FOUNTAIN_RADIUS+PARTICLE_SIZE/2)

void UpdateParticle (PARTICLE *p, float dt)
{
    // If the particle is not active, we need not do anything
    if (!p->active) return;

    // The particle is getting older...
    p->life = p->life - dt * (1.0f / LIFE_SPAN);

    // Did the particle die?
    if( p->life <= 0.0f ) {
        p->active = 0;
        return;
    }

    // Update particle velocity (apply gravity)
    p->vz = p->vz - GRAVITY * dt;

    // Update particle position
    p->x = p->x + p->vx * dt;
    p->y = p->y + p->vy * dt;
    p->z = p->z + p->vz * dt;

    // Simple collision detection + response
    if( p->vz < 0.0f )
    {
        // Particles should bounce on the fountain (with friction)
        if( (p->x*p->x + p->y*p->y) < FOUNTAIN_R2 &&
            p->z < (FOUNTAIN_HEIGHT + PARTICLE_SIZE/2) )
        {
            p->vz = -FRICTION * p->vz;
            p->z  = FOUNTAIN_HEIGHT + PARTICLE_SIZE/2 +
                        FRICTION * (FOUNTAIN_HEIGHT +
                        PARTICLE_SIZE/2 - p->z);
        }

        // Particles should bounce on the floor (with friction)
        else if( p->z < PARTICLE_SIZE/2 )
        {
            p->vz = -FRICTION * p->vz;
            p->z  = PARTICLE_SIZE/2 +
                        FRICTION * (PARTICLE_SIZE/2 - p->z);
        }

    }
}

void ParticleEngine( float t, float dt )
{
    int      i;
    float    dt2;

    // Update particles (iterated several times per frame if dt is too
    // large)
    while( dt > 0.0f ) {
        // Calculate delta time for this iteration
        dt2 = dt < MIN_DELTA_T ? dt : MIN_DELTA_T;

        // Update particles
        for( i = 0; i < MAX_PARTICLES; i ++ )  {
            UpdateParticle( &particles[ i ], dt2 );
        }

        // Increase minimum age
        min_age += dt2;

        // Should we create any new particle(s)?
        while( min_age >= BIRTH_INTERVAL ) {
            min_age -= BIRTH_INTERVAL;

            // Find a dead particle to replace with a new one
            for( i = 0; i < MAX_PARTICLES; i ++ ) {
                if( !particles[ i ].active ) {
                    InitParticle( &particles[ i ], t + min_age );
                    UpdateParticle( &particles[ i ], min_age );
                    break;
                }
            }
        }
        // Decrease frame delta time
        dt -= dt2;
    }
}

#define FtoRGB24(r,g,b) ((int)((r)*255.0)<<16|(int)((g)*255.0)<<8|(int)((b)*255.0))

int zpsort (const void *a, const void *b)
{
	const TZPOINT *p1 = (TZPOINT*)a;
	const TZPOINT *p2 = (TZPOINT*)b;

	if (p1->z > p2->z)
		return 1;
	else
		return -1;
}

void DrawParticles (TFRAME *frame, float t, float dt)
{
	ParticleEngine (t, dt);
    
    int sx=0,sy=0,i;
    float x,y,z;
	float x2,y2,z2;
	float tmp;
    const PARTICLE *pptr = particles;
	const float ThetaX = /*xpos*/-53.0 * DEGTORAD; //xpos * DEGTORAD;
	const float ThetaY = ypos * DEGTORAD;
	const float ThetaZ = zpos * DEGTORAD;
	const float cosx = cosf(ThetaX);
	const float cosy = cosf(ThetaY);
	const float cosz = cosf(ThetaZ);
	const float sinx = sinf(ThetaX);
	const float siny = sinf(ThetaY);
	const float sinz = sinf(ThetaZ);

    for (i = MAX_PARTICLES; --i; ){
        if (pptr->active){
			x = pptr->x;
			z = pptr->z;
			y = pptr->y;
			
  			//RotateX (ThetaX, &y, &z);
  			y2 = (y*cosx) - (z*sinx);
			z = (z*cosx) + (y*sinx);
			y = y2;
				
  			//RotateY (ThetaY, &x, &z);
  			z2 = (z*cosy) - (x*siny);
			x = (x*cosy) + (z*siny);
			z = z2;
				
   			//RotateZ (ThetaZ, &x, &y);
			x2 = (x*cosz) - (y*sinz);
			y = (y*cosz) + (x*sinz);
			x = x2;

			if (z-zoom >= 0.0f){
				tmp = flength/(z-zoom);
				sx = (x*tmp)+destx;
				sy = (-y*tmp)+desty;

				points[i].z = tmp;
				points[i].x = sx;
				points[i].y = sy;
				points[i].c = 0xFF000000|FtoRGB24(pptr->r, pptr->g, pptr->b);
				//lSetPixel(frame, sx,sy, LSP_SET);
				//lSetPixelf(frame, sx, sy, pptr->r, pptr->g, pptr->b);
				//lDrawCircleFilled(frame, sx, sy, 1.5, FtoRGB(pptr->r, pptr->g, pptr->b));
				points[i].active = 0;
			}else{
				points[i].active = 1;
			}
        }
        pptr++;
    }
    
    // sort z
	qsort(points, MAX_PARTICLES, sizeof(TZPOINT), zpsort);

	TZPOINT *p = points;
    for (i = MAX_PARTICLES; --i; p++){
    	if (!p->active)
    		lDrawCircleFilled(frame, p->x, p->y, 3.0, p->c);
    }
}

void Draw (TFRAME *frame, float t)
{
    static float t_old = 0.0;
    static float  dt;

    // Calculate frame-to-frame delta time
    
    dt = t-t_old;
    t_old = t;


    // Rotate camera
    angle_x = 90.0 - 10.0;
    angle_y = 40.0 * sinf( 0.3 * t );
    angle_z = 10.0 * t;

    // Translate camera
    xpos =  15.0 * sinf( (M_PI/180.0) * angle_z ) + 2.0 * sinf( (M_PI/180.0) * 3.1 * t );
    ypos = -15.0 * cosf( (M_PI/180.0) * angle_z ) + 2.0 * cosf( (M_PI/180.0) * 2.9 * t );
    zpos = 4.0 + 2.0 * cosf( (M_PI/180.0) * 4.9 * t );

    //xpos = 20.0 * cos( 0.3 * t );
    //ypos = 4.0 + 1.0 * sin( 1.0 * t );
    //zpos = 20.0 * sin( 0.3 * t );

    lClearFrame(frame);
    //DrawFloor(frame);
    DrawParticles(frame,t,dt);
	lRefresh(frame);
}

//#define PRId64 I64d

int main ()
{
	if (!initDemoConfig("config.cfg"))
		return 0;

	hw->render->backGround = lGetRGBMask(frame, LMASK_BLACK);
	hw->render->foreGround = lGetRGBMask(frame, LMASK_WHITE);
	lClearFrame(frame);
	
	uint64_t freq, t0_64, t_64;
    int i;

 	destx = DWIDTH/2;
 	desty = (DHEIGHT/2)+8;

    for(i= 0; i < MAX_PARTICLES; i ++ )
        particles[i].active = 0;

	QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

	float Resolution = 1.0 / (float)freq;
	
	//printf("%I64d %I64d %f\n", freq, (uint64_t)(Resolution*1000000000000.0), ((float)(t_64 - t0_64))*Resolution);
	
	QueryPerformanceCounter((LARGE_INTEGER *)&t0_64);

    while(!kbhit()) {
		QueryPerformanceCounter((LARGE_INTEGER *)&t_64);
		Draw(frame, ((float)(t_64 - t0_64))*Resolution);
	}
    
	demoCleanup();

    return 0;
}

