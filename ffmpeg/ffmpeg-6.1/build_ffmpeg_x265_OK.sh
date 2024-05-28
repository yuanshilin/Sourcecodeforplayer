#
#Using gdb to debug fmpeg_g : add --enable-debug=3 --disable-optimizations --disable-asm --disable-stripping  as
#below, gdb ffmpeg_g, then set args -i 2.mp4 2.avi
#https://www.jianshu.com/p/0551d7657ed0
#cd ~/ffmpeg_sources &&  \
#git clone ~/ffmpeg_sources_orig/ffmpeg/ && \
#cd ffmpeg && \
PATH="$HOME/bin:$PATH" PKG_CONFIG_PATH="$HOME/ffmpeg_build/lib/pkgconfig" ./configure \
	--prefix="$HOME/ffmpeg_build" \
	--pkg-config-flags="--static" \
	--extra-cflags="-I$HOME/ffmpeg_build/include -DX265_DEPTH=10" \
	--extra-ldflags="-L$HOME/ffmpeg_build/lib" \
	--extra-libs="-lpthread -lm" \
	--ld="g++" \
	--bindir="$HOME/bin" \
	--enable-gpl \
	--disable-asm \
	--enable-gnutls \
	--enable-libfreetype \
	--enable-libx265 && \
	PATH="$HOME/bin:$PATH" make && \
	hash -r
