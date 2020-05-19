{
  'targets': [
    {
      "target_name": "none",
      "sources": [],
    }
  ],
  'conditions': [
    ['OS=="mac"', {
      'targets': [
        {
          "target_name": "smc",
          "sources": [ "lib/OSX/smc-read.h", "lib/OSX/smc-read.cc" ],
          "link_settings": {
                  'libraries': [
                    'IOKit.framework'
                  ]
           }
        }
      ],
    }]
  ]
}
