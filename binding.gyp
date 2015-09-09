{
  'targets': [
    {
      'target_name': 'nwm',
      'sources': [
        'src/nwm/nwm_node.cc'
      ],
      'dependencies': ['listc', 'nwmc'],
      'include_dirs' : [
          "<!(node -e \"require('nan')\")"
      ]
    },
    {
      'target_name': 'listc',
      'type': 'static_library',
      'sources': [
        'src/nwm/list.c'
      ],
      'cflags': ['-fPIC', '-std=c99', '-pedantic', '-Wall']
    },
    {
      'target_name': 'nwmc',
      'type': 'static_library',
      'sources': [
        'src/nwm/nwm.c'
      ],
      'cflags': ['-fPIC', '-std=c99', '-pedantic', '-Wall'],
      'link_settings': {
        'libraries': [
          '-lX11', '-lXinerama'
        ],
      }
    }
  ]
}
