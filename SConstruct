# build
lib_path = [
	     '/home/xudong.yang/AliWS-1.4.0.0/lib',
             '/usr/local/lib',
             '/usr/local/lib64',
             '/usr/lib',
             '/usr/lib64',
           ]

include_path = [ '/usr/include',
                 '/usr/local/include',
	         '/home/xudong.yang/AliWS-1.4.0.0/include'
               ]

env = Environment(
    LIBPATH = lib_path,
    LIBS = [ File('/home/xudong.yang/AliWS-1.4.0.0/lib/libAliWS.a'), File('/usr/lib64/libexpat.a'), 'pthread', 'dl'],
    CPPPATH = include_path,
    CXXFLAGS = ['-ggdb', '-Wall', '-fPIC', '-O2', '-D__STDC_LIMIT_MACROS'],
)

lib_sources = [ Glob('*.cpp') ]
env.Program('segment', lib_sources)
