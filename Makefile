6818:main.c myjpeg.c lcd_app.c camera.c ts_slide.c 
	arm-linux-gcc main.c myjpeg.c lcd_app.c camera.c ts_slide.c -o 6818 -I ./include/ -I ./include/freetype2/ -L ./lib/ -ljpeg -lfreetype -lts -lm -pthread
