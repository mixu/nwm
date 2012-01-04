import sys

srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
  opt.tool_options("compiler_cxx")
  opt.tool_options('compiler_cc')

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")
  conf.check_tool("compiler_cc")

def build(bld):
  # I don't really understand the waf build system, but this works :p
  # bld.all_envs['default'] = bld.all_envs['Release']
  # list.c
  listlib = bld.new_task_gen('cc')
  listlib.source = 'src/list/list.c'
  listlib.includes = [ './include/list' ]
  listlib.cflags = ['-std=c99', '-pedantic', '-Wall', '-fPIC']

  # nwm.c
  nwmlib = bld.new_task_gen('cc')
  nwmlib.source = 'src/nwm/nwm.c'
  nwmlib.includes = [ './include/nwm', './include/list' ]
  nwmlib.cflags = ['-std=c99', '-pedantic', '-Wall', '-fPIC']
  if sys.platform.startswith("darwin"):
    nwmlib.linkflags=[ '-L/usr/X11/lib']
  else:
    nwmlib.framework=['X11','Xinerama']
  nwmlib.lib=['X11', 'Xinerama']

  # nwm_node.cc
  if sys.platform.startswith("darwin"):
    nwm = bld.new_task_gen('cxx', 'shlib', 'node_addon')
    nwm.linkflags=[ '-L/usr/X11/lib']
  else:
    nwm = bld.new_task_gen('cxx', 'shlib', 'node_addon')
    nwm.framework = ['X11', 'Xinerama']
  nwm.lib=['X11', 'Xinerama']
  nwm.cxxflags = ["-g", "-static", "-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE", "-Wall"]
  nwm.includes = [ './include/nwm', './include/list' ]
  nwm.target = "nwm"
  nwm.source = "src/nwm/nwm_node.cc"
  bld.env.append_value('LINKFLAGS', [
    bld.srcnode.abspath(bld.env)+'src/list/list_1.o',
    bld.srcnode.abspath(bld.env)+'src/nwm/nwm_2.o',
  ])
