#include <android/log.h>
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <errno.h>
#include <GLES2/gl2.h>
#include <android/sensor.h>

struct GameData
{
	struct android_app	*pApp;
	EGLDisplay			Display;
	EGLSurface			Surface;
	EGLContext			Context;
	bool				Animating;
	int					Width;
	int					Height;
	int					X;
	int					Y;
};

static int InitialiseDisplay( struct GameData *p_pGameData );
static void DrawFrame( struct GameData *p_pGameData );
static void TerminateDisplay( struct GameData *p_pGameData );
static int32_t HandleInput( struct android_app *p_pApp,
	AInputEvent *p_pEvent );
static void HandleCmd( struct android_app *p_pApp, int32_t p_Cmd );

void android_main( struct android_app *p_pState )
{
	app_dummy( );

	struct GameData Game;
	memset( &Game, 0, sizeof( Game ) );

	Game.pApp = p_pState;

	p_pState->userData = &Game;
	p_pState->onAppCmd = HandleCmd;
	p_pState->onInputEvent = HandleInput;

	while( 1 )
	{
		int Events;
		struct android_poll_source *pSource;

		while( ( ALooper_pollAll( Game.Animating ? 0 : -1, NULL, &Events,
			( void ** )&pSource ) ) >= 0 )
		{
			if( pSource != NULL )
			{
				pSource->process( p_pState, pSource );
			}

			if( p_pState->destroyRequested != 0 )
			{
				TerminateDisplay( &Game );
				return;
			}
		}

		if( Game.Animating )
		{
			DrawFrame( &Game );
		}
	}
}

int InitialiseDisplay( struct GameData *p_pGameData )
{
	EGLDisplay Display = eglGetDisplay( EGL_DEFAULT_DISPLAY );

	eglInitialize( Display, 0, 0 );

	const EGLint Attributes[ ]
	{
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE,	8,
		EGL_GREEN_SIZE,	8,
		EGL_BLUE_SIZE,	8,
		EGL_ALPHA_SIZE,	8,
		EGL_NONE
	};

	EGLConfig Config;
	EGLint NumConfigs;
	eglChooseConfig( Display, Attributes, &Config, 1, &NumConfigs );

	EGLint Format;
	eglGetConfigAttrib( Display, Config, EGL_NATIVE_VISUAL_ID, &Format );

	EGLSurface Surface = eglCreateWindowSurface( Display, Config,
		p_pGameData->pApp->window, NULL );
	EGLContext Context = eglCreateContext( Display, Config, NULL, NULL );

	if( eglMakeCurrent( Display, Surface, Surface, Context ) == EGL_FALSE )
	{
		return 1;
	}

	EGLint Width, Height;
	eglQuerySurface( Display, Surface, EGL_WIDTH, &Width );
	eglQuerySurface( Display, Surface, EGL_HEIGHT, &Height );

	p_pGameData->Display = Display;
	p_pGameData->Context = Context;
	p_pGameData->Surface = Surface;
	p_pGameData->Width = Width;
	p_pGameData->Height = Height;

	glEnable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );

	return 0;
}

void DrawFrame( struct GameData *p_pGameData )
{
	if( p_pGameData->Display == NULL )
	{
		return;
	}

	float Red = 184.0f / 255.0f;
	glClearColor( Red, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );
	eglSwapBuffers( p_pGameData->Display, p_pGameData->Surface );
}

void TerminateDisplay( struct GameData *p_pGameData )
{
	if( p_pGameData->Display != EGL_NO_DISPLAY )
	{
		eglMakeCurrent( p_pGameData->Display, EGL_NO_SURFACE, EGL_NO_SURFACE,
			EGL_NO_CONTEXT );
		
		if( p_pGameData->Context != EGL_NO_CONTEXT )
		{
			eglDestroyContext( p_pGameData->Display, p_pGameData->Context );
		}

		if( p_pGameData->Surface != EGL_NO_SURFACE )
		{
			eglDestroySurface( p_pGameData->Display, p_pGameData->Surface );
		}

		eglTerminate( p_pGameData->Display );
	}

	p_pGameData->Animating = false;
	p_pGameData->Display = EGL_NO_DISPLAY;
	p_pGameData->Surface = EGL_NO_SURFACE;
	p_pGameData->Context = EGL_NO_CONTEXT;
}

int32_t HandleInput( struct android_app *p_pApp, AInputEvent *p_pEvent )
{
	struct GameData *pGameData = ( struct GameData * )p_pApp->userData;

	if( AInputEvent_getType( p_pEvent ) == AINPUT_EVENT_TYPE_MOTION )
	{
		int32_t Action = AMotionEvent_getAction( p_pEvent );

		if( Action != AMOTION_EVENT_ACTION_MOVE )
		{
			pGameData->Animating = false;
			return 1;
		}

		pGameData->X = AMotionEvent_getX( p_pEvent, 0 );
		pGameData->Y = AMotionEvent_getY( p_pEvent, 0 );
		pGameData->Animating = true;

		return 1;
	}

	return 0;
}

void HandleCmd( struct android_app *p_pApp, int32_t p_Cmd )
{
	struct GameData *pGameData = ( struct GameData * )p_pApp->userData;

	switch( p_Cmd )
	{
		case APP_CMD_INIT_WINDOW:
		{
			if( pGameData->pApp->window != NULL )
			{
				InitialiseDisplay( pGameData );
				DrawFrame( pGameData );
			}
			break;
		}
		case APP_CMD_TERM_WINDOW:
		{
			TerminateDisplay( pGameData );
			break;
		}
		case APP_CMD_LOST_FOCUS:
		{
			pGameData->Animating = false;
			break;
		}

	}
}

