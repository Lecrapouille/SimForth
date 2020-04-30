\ =============================================================
\ Minimal OpenGL example inspired by
\ https://github.com/Lecrapouille/GraphicsLessonInGforth
\ =============================================================

\ -------------------------------------------------------------
\ Import c functions of OpenGL and SDL
C-LIB libopengl

ADD-LIB GL
ADD-LIB GLU
ADD-LIB SDL

\C #if defined(__APPLE__) && defined(__MACH__)
\C      #include <OpenGL/gl.h>
\C      #include <OpenGL/glu.h>
\C #else
\C      #include <GL/gl.h>
\C      #include <GL/glu.h>
\C #endif
\C #include <SDL/SDL.h>

\C void GetError() { printf("%s\n", SDL_GetError()); }

C-FUNCTION gl-clear             glClear                      i
C-FUNCTION gl-clear-color       glClearColor           f f f f
C-FUNCTION gl-clear-depth       glClearDepth                 f
C-FUNCTION gl-enable            glEnable                     i
C-FUNCTION gl-depth-func        glDepthFunc                  i
C-FUNCTION gl-hint              glHint                     i i
C-FUNCTION gl-load-identity     glLoadIdentity
C-FUNCTION gl-matrix-mode       glMatrixMode                 i
C-FUNCTION gl-shade-model       glShadeModel                 i
C-FUNCTION gl-viewport          glViewport             i i i i
C-FUNCTION glu-perspective      gluPerspective         f f f f

c-function gl-begin            glBegin                      i
c-function gl-end              glEnd
c-function gl-translate-f      glTranslatef             f f f
c-function gl-vertex-3f        glVertex3d               f f f

C-FUNCTION sdl-init		SDL_Init		i -- i
C-FUNCTION sdl-quit		SDL_Quit
C-FUNCTION sdl-gl-swap-buffers  SDL_GL_SwapBuffers
C-FUNCTION sdl-gl-set-attribute SDL_GL_SetAttribute        i i
C-FUNCTION sdl-set-video-mode	SDL_SetVideoMode       i i i i -- a
c-function sdl-getvideoinfo     SDL_GetVideoInfo               -- a
c-function sdl-geterror         GetError

END-C-LIB

\ -------------------------------------------------------------
\ OpenGL constants
: GL_SMOOTH $1D01 ;
: GL_DEPTH_TEST $0B71 ;
: GL_LEQUAL $0203 ;
: GL_PERSPECTIVE_CORRECTION_HINT $0C50 ;
: GL_NICEST $1102 ;
: GL_COLOR_BUFFER_BIT $00004000 ;
: GL_DEPTH_BUFFER_BIT $00000100 ;
: GL_PROJECTION $1701 ;
: GL_MODELVIEW $1700 ;
: GL_TRIANGLES $0004 ;
: GL_QUADS $0007 ;

\ -------------------------------------------------------------
\ SDL constants
: SDL_INIT_EVERYTHING $0000FFFF ;
: SDL_HWPALETTE $20000000 ;
: SDL_SWSURFACE $00000000 ;
: SDL_HWSURFACE $00000001 ;
: SDL_OPENGL $00000002 ;
: SDL_GL_DOUBLEBUFFER 5 ;

\ -------------------------------------------------------------
\ Application constants
: screen-width 640 ;
: screen-height 480 ;
: screen-bpp 16 ;

\ -------------------------------------------------------------
\ Application variables
0 VALUE videoflags
0 VALUE videoinfo
0 VALUE screen
FALSE value opengl-exit-flag

\ -------------------------------------------------------------
\ Print a colorful error message
: Error ( -- )
   TERM.STYLE.BOLD TERM.FG.YELLOW TERM.COLOR
   ." [ERROR] " TYPE sdl-geterror
   TERM.RESET.COLOR
   BYE
;

\ -------------------------------------------------------------
\ Initialize the SDL Video subsystem
: Initialize-SDL ( -- )
   SDL_INIT_EVERYTHING sdl-init 0< IF
      s" Video Initialization failed: " Error
   ELSE
      ." SDL init" CR
   ENDIF
;

\ -------------------------------------------------------------
\ Load information about the video hardware in the computer
\ : Get-Video-Info ( -- )
\    sdl-getvideoinfo dup videoinfo ! 0= IF
\       ." Video query failed: " sdl-geterror
\       BYE
\    ENDIF
\ ;

\ -------------------------------------------------------------
\ Build a flag variable specifying the video characteristics
\ to set
: Compile-Video-Flags ( -- )
   SDL_OPENGL                             \ enable OpenGL in SDL
   SDL_GL_DOUBLEBUFFER OR              \ Enable double buffering
   SDL_HWPALETTE OR              \ Store the palette in hardware
   \ SDL_RESIZABLE OR                   \ Enable window resizing
   SDL_SWSURFACE OR \ FIXME qq
   videoflags !                                 \ save the flags
;

\ Add flag for if hardware surfaces can be created
\ : Check-HW-Surfaces ( -- )
\   videoinfo sdl-video-info-hw-available @ 0<> if
\     SDL_HWSURFACE
\   else
\     SDL_SWSURFACE
\   then
\   videoflags OR videoflags !
\ ;

\ Add flag for if hardware-to-hardware blits are available
\ : Check-HW-Blits ( -- )
\   videoinfo sdl-video-info-blit-hw @ 0<> if
\     videoflags SDL_HWACCEL OR videoflags !
\   then
\ ;

\ -------------------------------------------------------------
\ Enable double buffering
: Init-Double-Buffering ( -- )
   SDL_GL_DOUBLEBUFFER 1 sdl-gl-set-attribute
;

\ -------------------------------------------------------------
\ Create an SDL surface and open the display window
: Init-Video ( -- )
   screen-width screen-height screen-bpp videoflags
      sdl-set-video-mode
   DUP 0= IF
      DROP
      s" Video mode set failed: " Error
   ENDIF
   screen !
;

: InitGL ( -- boolean )
   \ Enable smooth shading
   GL_SMOOTH gl-shade-model
   \ Set the background black
   0.0 0.0 0.0 0.0 gl-clear-color
   \ Depth buffer setup
   1.0 gl-clear-depth
   \ Enable depth testing
   GL_DEPTH_TEST gl-enable
   \ Type of depth test to do
   GL_LEQUAL gl-depth-func
   \ Really nice perspective calculations
   GL_PERSPECTIVE_CORRECTION_HINT GL_NICEST gl-hint
   \ Return a good value
   TRUE
;

: ResizeWindow ( -- boolean )
   \ set up the viewport
   0 0 screen-width screen-height gl-viewport
   \ Change to the projection matrix and set our viewing volume
   GL_PROJECTION gl-matrix-mode
   \ Reset the matrix
   gl-load-identity
   \ Set our perspective - the F/ calcs the aspect ratio of w/h
   45.0 screen-width >FLOAT screen-height >FLOAT F/ 0.1 100.0 glu-perspective
   \ Make sure we are changing the model view and not the projection
   GL_MODELVIEW gl-matrix-mode
   \ Reset the matrix
   gl-load-identity
   \ Return a good value
   TRUE
;

: DrawGLScene ( -- boolean )
   \ Clear the screen and the depth buffer
   GL_COLOR_BUFFER_BIT GL_DEPTH_BUFFER_BIT OR gl-clear
   \ Reset the matrix
   gl-load-identity

   \ Move left 1.5 units, and into the screen 6.0
   -1.5 0.0 -6.0 gl-translate-f
   GL_TRIANGLES gl-begin                \ drawing using triangles
     0.0  1.0 0.0 gl-vertex-3f                              \ top
    -1.0 -1.0 0.0 gl-vertex-3f                      \ bottom left
     1.0 -1.0 0.0 gl-vertex-3f                     \ bottom right
   gl-end                         \ finished drawing the triangle

   \ Move right 3 units
   3.0 0.0 0.0 gl-translate-f
   GL_QUADS gl-begin                                \ draw a quad
    -1.0  1.0 0.0 gl-vertex-3f                         \ top left
     1.0  1.0 0.0 gl-vertex-3f                        \ top right
     1.0 -1.0 0.0 gl-vertex-3f                     \ bottom right
    -1.0 -1.0 0.0 gl-vertex-3f                      \ bottom left
   gl-end

   \ Draw it to the screen -- if double buffering is permitted
   sdl-gl-swap-buffers

   \ Gather  our frames per second count
   \ fps-frames 1+ to fps-frames
   \ Display the FPS count to the terminal window
   \ Display-FPS
   \ Return a good value
   TRUE
;

: HelloOpenGL ( -- )
   ." Hello OpenGL"
   Initialize-SDL
   Compile-Video-Flags
   Init-Double-Buffering
   Init-Video
   \ Init-Caption
   InitGL FALSE == IF
      sdl-quit
      CR ." Could not initialize OpenGL." CR
      BYE
   ENDIF
   ( screen-width screen-height ) ResizeWindow DROP

   BEGIN
     DrawGLScene
     opengl-exit-flag 0=
   UNTIL
;

HelloOpenGL
