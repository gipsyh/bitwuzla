# Names of unit tests without test_ prefix and .cpp suffix
unit_tests = [
  ['api',
    [
      'api',
      'capi'
    ]
  ],

  ['backtrack',
    [
      'assertion_stack',
      'object',
      'unordered_map',
      'unordered_set',
      'vector'
    ]
  ],

  ['node',
    [
      'node',
      'node_manager',
      'node_utils'
    ]
  ],

  ['option',
    [
      'options'
    ]
  ],

  ['parser',
    [
      'btor2_lexer',
      'smt2_lexer',
      'smt2_symbol_table',
    ]
  ],
  ['preprocess',
    [
      'assertion_tracker',
      'pass_contradicting_ands',
      'pass_normalize',
      'pass_flatten_and',
      'pass_variable_substitution',
      'preprocessor',
    ]
  ],

  ['printer',
    [
      'printer'
    ]
  ],

  ['rewrite',
    [
      'rewriter_core',
      'rewriter_utils',
      'rewriter_bool',
      'rewriter_bv',
      'rewriter_bv_norm',
      'rewriter_bv_overflow',
      'rewriter_bv_rotate',
      'rewriter_fp',
    ]
  ],

  ['solver',
    [
      'fun_solver',
      'incremental',
      'bv_solver',
      'bv_prop_solver',
      'fp_solver',
      'fp_floating_point',
    ]
  ],

  ['type',
    [
      'type_manager'
    ]
  ],

  ['lib/bitblast',
    [
      'aig_bitblaster',
      'aig_manager',
      'aig_cnf'
    ]
  ],

  ['lib/bitvector',
    [
      'bv',
      'bvdomain',
      'bvdomaingen',
    ]
  ],

  ['lib/ls/bv',
    [
      'bvnode',
      'bvnodecons',
      'bvnodeinv',
      'bvnodeiscons',
      'bvnodeisess',
      'bvnodeisinv',
      'bvnodeselpath',
      'local_search_bv',
      ]
  ],

  ['util',
    [
      'integer',
    ]
  ],

]

gtest_dep = dependency('gtest_main', required: true)
test_inc = [include_directories('../../src', '../..', 'lib')]
test_deps = [gtest_dep, bitwuzla_dep, gmp_dep]

# Disable wine debug output if we run the cross-compiled binary
env = []
if meson.is_cross_build() and host_machine.system() == 'windows'
  env += ['WINEDEBUG=fixme-all']
endif

cpp_args = ['-fno-access-control']
if cms_dep.found()
  cpp_args += ['-DBZLA_USE_CMS']
endif
if kissat_dep.found()
  cpp_args += ['-DBZLA_USE_KISSAT']
endif
if get_option('fpexp')
  cpp_args += ['-DBZLA_USE_FPEXP']
endif

# Add unit tests
foreach p : unit_tests
  test_subdir = p[0]
  suite = test_subdir.replace('/', '_')
  foreach name : p[1]
    basename = ''.join('test_', name, '.cpp')
    src = join_paths(test_subdir, basename)
    name = '_'.join(suite, name)
    exename = ''.join('test_', name)

    exe = executable(exename, src,
               dependencies: test_deps,
               include_directories: test_inc,
               cpp_args: cpp_args)
    test(name, exe, suite: ['unit', suite], timeout: 0, env: env)
  endforeach
endforeach
