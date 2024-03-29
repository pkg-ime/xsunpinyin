import os

sources = ['ic.c',
           'ic_gtk.c',
           'ic_skin.c',
           'skin.c',
           'ui.c',
           'main.c',
           'settings.c',
           'sunpinyin_preedit.cc',
           'sunpinyin_preedit_gtk.cc',
           'sunpinyin_preedit_skin.cc',
           'xim.c',
           'xim_trigger.c',
           'xmisc.c']

preferences_sources = ['preferences.c',
                       'settings.c',
                       'xmisc.c']

imdkit_sources = ['IMdkit/FrameMgr.c',
                  'IMdkit/i18nAttr.c',
                  'IMdkit/i18nClbk.c',
                  'IMdkit/i18nIc.c',
                  'IMdkit/i18nIMProto.c',
                  'IMdkit/i18nMethod.c',
                  'IMdkit/i18nPtHdr.c',
                  'IMdkit/i18nUtil.c',
                  'IMdkit/i18nX.c',
                  'IMdkit/IMConn.c',
                  'IMdkit/IMMethod.c',
                  'IMdkit/IMValues.c']


cflags='-O2 -g -pipe'

# options
AddOption('--prefix', dest='prefix', type='string', nargs=1, action='store',
          metavar='DIR', help='installation prefix')
AddOption('--rpath', dest='rpath', type='string', nargs=1, action='store',
          metavar='DIR', help='encode rpath in the executables')

# save the options
opts = Variables('configure.conf')
opts.Add('PREFIX', default='/usr/local')

def PassVariables(envvar, env):
    for (x, y) in envvar:
        if x in os.environ:
            print 'Warning: you\'ve set %s in the environmental variable!' % x
            env[y] = os.environ[x]

env = Environment(ENV=os.environ,
                  CFLAGS=cflags, CXXFLAGS=cflags,
                  LINKFLAGS=['-export-dynamic'],
                  CPPPATH=['.', 'IMdkit'])
opts.Update(env)


if GetOption('prefix') is not None:
    env['PREFIX'] = GetOption('prefix')

opts.Save('configure.conf', env)


# set rpath
if GetOption('rpath') is not None:
    env.Append(LINKFLAGS='-Wl,-R -Wl,%s' % GetOption('rpath'))

envvar = [('CC', 'CC'),
          ('CXX', 'CXX'),
          ('CFLAGS', 'CFLAGS'),
          ('CXXFLAGS', 'CXXFLAGS'),
          ('LDFLAGS', 'LINKFLAGS')]
PassVariables(envvar, env)

bin_dir = env['PREFIX'] + '/bin'
data_dir = '%s/share/xsunpinyin/' % env['PREFIX']
icon_dir = data_dir + 'icons/'

extra_cflags =  ' -DSUNPINYIN_XIM_ICON_DIR=\\"%s\\"' % icon_dir
extra_cflags += ' -DSUNPINYIN_XIM_SETTING_DIR=\\"%s\\"' % data_dir

env.Append(CFLAGS=extra_cflags)
env.Append(CXXFLAGS=extra_cflags)


#
#==============================configure================================
#
def CheckPKGConfig(context, version='0.12.0'):
    context.Message( 'Checking for pkg-config... ' )
    ret = context.TryAction('pkg-config --atleast-pkgconfig-version=%s' % version)[0]
    context.Result(ret)
    return ret

def CheckPKG(context, name):
    context.Message( 'Checking for %s... ' % name )
    ret = context.TryAction('pkg-config --exists \'%s\'' % name)[0]
    context.Result(ret)
    return ret

conf = Configure(env, custom_tests={'CheckPKGConfig' : CheckPKGConfig,
                                    'CheckPKG' : CheckPKG })
def DoConfigure():
    if GetOption('clean'):
        return

    if not conf.CheckPKGConfig():
        Exit(1)

    if not conf.CheckPKG('gtk+-2.0'):
        Exit(1)

    if not conf.CheckPKG('x11'):
        Exit(1)

    if not conf.CheckPKG('sunpinyin-2.0'):
        Exit(1)

    env = conf.Finish()
    env.ParseConfig('pkg-config gtk+-2.0 x11 sunpinyin-2.0 --libs --cflags')

DoConfigure()

env.Append(LIBS=env.Library('IMdkit', source=imdkit_sources))
env.Program('xsunpinyin', source=sources)
env.Program('xsunpinyin-preferences', source=preferences_sources)


def DoInstall():
    bin_target = env.Install(bin_dir,
                             ['xsunpinyin', 'xsunpinyin-preferences'])
    icon_target = env.Install(icon_dir,
                              ['data/chnpunc.png',
                               'data/han.png',
                               'data/eng.png',
                               'data/han.svg',
                               'data/eng.svg',
                               'data/engpunc.png',
                               'data/full.png',
                               'data/half.png',
                               'data/sunpinyin-logo-big.png',
                               'data/sunpinyin-logo.png'])
    data_target = env.Install(data_dir, ['data/settings_ui.xml'])
    data_target += env.Install(data_dir + 'skins/', Glob('data/skins/*'))
    env.Alias('install-bin', bin_target)
    env.Alias('install-data', [icon_target, data_target])

DoInstall()
env.Alias('install', ['install-bin', 'install-data'])


