import ConfigParser

cfg = ConfigParser.ConfigParser()
cfg.read("build.properties")
DEBUG		= cfg.getboolean("build", "DEBUG")

toolsEnv = Environment()
toolsEnv.Append( LIBS=['opencv_core', 'opencv_imgproc', 'opencv_highgui', 'opencv_video'])
toolsEnv.Append( LIBS=['avcodec', 'avutil', 'avformat', 'swscale','m', 'pthread'])
#toolsEnv.Append( LIBPATH='/home/stanislaw/private/msc/ffmpeg' )
#toolsEnv.Append( CPPPATH=['/home/stanislaw/private/msc/ffmpeg'] )
toolsEnv.Append( CPPFLAGS=['-std=c99'] )

if DEBUG:
        toolsEnv.Append( CPPFLAGS=['-ggdb'] )
else:
        toolsEnv.Append( CPPFLAGS=['-O2'] )

toolsEnv.Program('simple_similarity_comparator', ['src/tools/SimpleSimilarityComparator.cpp'] )
toolsEnv.Program('diff_viewer', ['src/diff_viewer.cpp'] )
toolsEnv.Program('diff_video', ['src/diff_video.c'] )

