C-LIB libgtk

PKG-CONFIG gtk+-2.0

\C #include <gtk/gtk.h>

C-FUNCTION gtk-main                     gtk_main
C-FUNCTION gtk-init                     gtk_init                       a a
C-FUNCTION gtk-window-new               gtk_window_new                   i -- a
C-FUNCTION gtk-widget-show-all          gtk_widget_show_all              a
C-FUNCTION gtk-widget-set-size-request  gtk_widget_set_size_request  a i i
C-FUNCTION gtk-window-set-position      gtk_window_set_position        a i
C-FUNCTION gtk-window-set-resizable     gtk_window_set_resizable       a i

C-FUNCTION gtk-image-new                gtk_image_new                      -- a
C-FUNCTION gtk-container-add            gtk_container_add              a a
C-FUNCTION gtk-widget-get-parent-window gtk_widget_get_parent_window     a -- a
C-FUNCTION gdk-pixmap-new               gdk_pixmap_new             a i i i -- a
C-FUNCTION gdk-gc-new                   gdk_gc_new                       a -- a
C-FUNCTION gtk-image-set-from-pixmap    gtk_image_set_from_pixmap    a a a
C-FUNCTION g-malloc                     g_malloc                         i -- a
C-FUNCTION gdk-color-parse              qq_gdk_color_parse             a a -- i
C-FUNCTION gdk-gc-set-rgb-fg-color      gdk_gc_set_rgb_fg_color        a a
C-FUNCTION gdk-draw-rectangle           gdk_draw_rectangle   a a i i i i i

END-C-LIB

0 value WIN
0 value IMAGE
0 value GDKWIN
0 value PIX
0 value GC
0 value COLOR

: GTK_WINDOW_TOPLEVEL 0 ;

0 0  gtk-init
GTK_WINDOW_TOPLEVEL gtk-window-new to WIN
WIN 300 350 gtk-widget-set-size-request
WIN 1 gtk-window-set-position
WIN 0 gtk-window-set-resizable
gtk-image-new to IMAGE
WIN IMAGE gtk-container-add
WIN gtk-widget-show-all

IMAGE gtk-widget-get-parent-window to GDKWIN
GDKWIN 300 350 -1 gdk-pixmap-new to PIX
PIX gdk-gc-new to GC
IMAGE PIX 0 gtk-image-set-from-pixmap
16 g-malloc to COLOR

z" #ffffff" COLOR gdk-color-parse
GC COLOR gdk-gc-set-rgb-fg-color
PIX GC 1 0 0 300 350 gdk-draw-rectangle

z" #ff0000" COLOR gdk-color-parse
GC COLOR gdk-gc-set-rgb-fg-color
PIX GC 1 0 0 100 100 gdk-draw-rectangle

gtk-main
