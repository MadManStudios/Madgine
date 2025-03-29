properties([pipelineTriggers([githubPush()])])

def toolchains = [
    "windows": [
			dockerImage : 'schuetzo/linux-test-env:latest',
			args : "-DCMAKE_TOOLCHAIN_FILE=../../cmake/toolchains/mingw.cmake -DGENERIC_COMPATIBILITY_CONFIG_HEADER=/Users/madman/compat/windows.h",
			artifacts : ['bin/*', 'data/*']
		],
	"osx": [			
			dockerImage : 'schuetzo/linux-test-env:latest',
			args : "-DENABLE_ARC=False -DDEPLOYMENT_TARGET=11.0 -DGENERIC_COMPATIBILITY_CONFIG_HEADER=/Users/madman/compat/osx.h",
			artifacts : ['bin/*', 'data/*']
		],
	"ios": [
			dockerImage : 'schuetzo/linux-test-env:latest',
			args : "-DCMAKE_TOOLCHAIN_FILE=../../cmake/toolchains/ios.cmake -DPLATFORM=SIMULATOR64 -DENABLE_ARC=False -DDEPLOYMENT_TARGET=11.0",
			artifacts : ['bin/*']
		],
	"linux": [
			dockerImage : 'schuetzo/linux-test-env:latest',
			args : "-DGENERIC_COMPATIBILITY_CONFIG_HEADER=/Users/madman/compat/linux.h",
			artifacts : ['bin/*', 'data/*']
		],
	"android": [
			dockerImage : 'schuetzo/linux-test-env:latest',
			args : "-DCMAKE_TOOLCHAIN_FILE=../../cmake/toolchains/android.cmake -DANDROID_ABI=x86_64 -DGENERIC_COMPATIBILITY_CONFIG_HEADER=/Users/madman/compat/android.h",
			artifacts : ['bin/*']
		],
	"emscripten": [			
			dockerImage : 'schuetzo/linux-test-env:latest',
			args : "-DCMAKE_TOOLCHAIN_FILE=../../cmake/toolchains/emscripten-wasm.cmake",
			artifacts : ['bin/*']
		]
]     

def configs = [
	"Plugins": [
			args: "-DBUILD_SHARED_LIBS=ON"
	],
	"OpenGL": [
			args: "-DMADGINE_CONFIGURATION=../../test/configs/OpenGL/ -DBUILD_SHARED_LIBS=OFF -DMADGINE_TOOLING_PRESET=clang-osx-Debug-Plugins"
	]
]


pipeline {
    agent any

	parameters {
        booleanParam(defaultValue: false, description: '', name: 'fullBuild')
		booleanParam(defaultValue: false, description: '', name: 'timeTrace')
		booleanParam(defaultValue: false, description: '', name: 'iwyu')
		booleanParam(defaultValue: false, description: '', name: 'taskTracker')
    }

	options{
		disableConcurrentBuilds()
	}

    stages {
		stage("checkout") {
			steps{
        		checkout scm
				sh """
					git submodule update --init --recursive
				"""
			}
	    }
		stage("cmake") {
			steps{
				script {				
					cmake_args = "-DUSE_CMAKE_LOG=1 "
					if (params.timeTrace){
						cmake_args = cmake_args + "-DCMAKE_CXX_FLAGS=-ftime-trace "
					}
					if (params.iwyu){
						cmake_args = cmake_args + """-DCMAKE_CXX_INCLUDE_WHAT_YOU_USE="/home/jenkins/tools/usr/local/bin/include-what-you-use;-Xiwyu;--pch_in_code;-Xiwyu;--prefix_header_includes=remove" """
					}
					if (params.taskTracker){
						cmake_args = cmake_args + "-DMODULES_ENABLE_TASK_TRACKING=ON "
					}
				}
				sh """
					mkdir -p build
					cd build
					if ${params.fullBuild}; then
						if [ -d "clang-osx-Debug-Plugins" ]; then 
							rm -Rf clang-osx-Debug-Plugins;
						fi
						if [ -d "util" ]; then 
							rm -Rf util;
						fi
					fi
					mkdir -p clang-osx-Debug-Plugins
					cd clang-osx-Debug-Plugins
					cmake ../.. \
					-DCMAKE_BUILD_TYPE=Debug \
					${toolchains.osx.args} \
					${configs.Plugins.args} \
					${cmake_args} > cmake.txt
					cat cmake.txt
				"""		
			}
		}
		stage("build") {	
			steps {
				sh """
					cd build
					cd clang-osx-Debug-Plugins
					make all
				"""				
			}
		}
		stage("Test") {
			steps {
				//docker.image(toolchain.dockerImage).inside {
					sh """
				#	cd clang-osx-Debug-Plugins
				#	ctest --output-on-failure
					"""
				//}
			}
		}   
		stage ("Doxygen") {
			steps {
				sh """
					cd build
					cd clang-osx-Debug-Plugins

					make doxygen
				"""
			}
		}
        stage ("Multiconfiguration Parallel Tasks") {
	        matrix {
				axes {
					axis {
						name 'PLATFORM'
						values 'windows', 'osx', 'ios', 'linux', 'android', 'emscripten'
					}
					axis {
						name 'TYPE'
						values 'Debug', 'RelWithDebInfo'
					}
					axis {
						name 'CONFIG'
						values 'Plugins', 'OpenGL'
					}
				}
				excludes {
                    exclude {
                        axis {
                            name 'CONFIG'
                            values 'Plugins'
                        }
                        axis {
                            name 'PLATFORM'
                            values 'ios', 'android', 'emscripten'
                        }
                    }
					exclude {
                        axis {
                            name 'PLATFORM'
                            values 'ios', 'android', 'emscripten'
                        }
					}
					exclude {
                        axis {
                            name 'CONFIG'
                            values 'Plugins'
                        }
                        axis {
                            name 'PLATFORM'
                            values 'osx'
                        }
						axis {
                            name 'TYPE'
                            values 'Debug'
                        }
                    }
                }
				stages {
					stage("cmake") {
						steps{
							sh """
								mkdir -p build
								cd build
								if ${params.fullBuild}; then
									if [ -d "clang-${PLATFORM}-${TYPE}-${CONFIG}" ]; then 
										rm -Rf clang-${PLATFORM}-${TYPE}-${CONFIG};
									fi
								fi
								mkdir -p clang-${PLATFORM}-${TYPE}-${CONFIG}
								cd clang-${PLATFORM}-${TYPE}-${CONFIG}
								cmake ../.. \
								-DCMAKE_BUILD_TYPE=${TYPE} \
								-DSKIP_UTIL_BUILD=ON \
								${toolchains[PLATFORM].args} \
								${configs[CONFIG].args} \
								${cmake_args} > cmake.txt
								cat cmake.txt
							"""		
						}
					}
					stage("build") {	
						steps {
							sh """
								cd build
								cd clang-${PLATFORM}-${TYPE}-${CONFIG}
								make all
							"""				
						}
					}
					stage("Test") {
						steps {
							//docker.image(toolchain.dockerImage).inside {
								sh """
							#	cd clang-${PLATFORM}-${TYPE}-${CONFIG}
							#	ctest --output-on-failure
								"""
							//}
						}
					}   
				}
	        }
        }
    }

	post {
        always {
			recordIssues enabledForFailure: true, tools: [clang()]

			//junit '**/*.xml'
		}
		success {
			sh """
				mkdir -p /opt/homebrew/var/www/${env.BRANCH_NAME}

				cp -r doc/* /opt/homebrew/var/www/${env.BRANCH_NAME}

				mkdir -p /opt/homebrew/var/www/${env.BRANCH_NAME}/live

				#cp build/clang-emscripten-RelWithDebInfo-OpenGL/bin/MadgineLauncher_plugins_tools.* /opt/homebrew/var/www/${env.BRANCH_NAME}/live
			"""
		}
    }
}
