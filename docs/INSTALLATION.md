## Overview
The page provides information on a simple approach to building, installing and utilizing the cxml library.   
<br/>

## Building cxml

You can build the library from sources using cmake. If you're using an IDE like CLion for example, this is as easy as clicking a button (with the right configurations in place). If you're building directly without the aid of an IDE, this can be done in a number of ways.

cxml is quite modular. It provides a couple of flags that can be used in the build process. This means that you can build cxml according to your own use-cases.

As mentioned in the [readme](https://github.com/ziord/cxml/blob/master/docs/README.md), cxml provides three interfaces that can be used to interact with an XML document, after it's parsed. These interfaces are not enforced on the user, and in fact the library can be built without any of them!

Here are some of the flags that can be used in the build command:

- `CXML_USE_QUERY_MOD`
This should be passed (with its value set to `ON`) if you need the query interface. The query interface provides a small and intuitive query language, as well as a number of functions for interacting with an XML document.
This flag is `OFF` by default.

<br/>

- `CXML_USE_SAX_MOD`
This should be passed (with its value set to `ON`) if you need the streaming (SAX) interface. This helps when parsing XML files that are too large to fit in memory at once. 
This flag is `OFF` by default.

<br/>

- `CXML_USE_XPATH_MOD`
This should be passed (with its value set to `ON`) if you need the XPATH interface. This allows you to interact with an XML document using the XPATH language. 
This flag is `OFF` by default.

<br/>

Other build flags:

- `CXML_BUILD_SHARED_LIB`
By default cxml builds as a static library. If you want to override this, you can supply this flag in the build command.
<br/>

- `CXML_BUILD_TESTS`
You can use this flag if you want to build the tests along with the library. This flag is `OFF` by default.
<br/>

**Building**

You can build the library using the steps below.

This builds cxml as a static library in release mode, along with all its interfaces as well as its tests:

```
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCXML_USE_QUERY_MOD=ON -DCXML_USE_SAX_MOD=ON -DCXML_USE_XPATH_MOD=ON -DCXML_BUILD_TESTS=ON ..
cmake --build .
```

Alternatively, you can build the library in debug mode by specifying the build type to be debug, that is:

```
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCXML_USE_QUERY_MOD=ON -DCXML_USE_SAX_MOD=ON -DCXML_USE_XPATH_MOD=ON -DCXML_BUILD_TESTS=ON ..
cmake --build .
```
<br/>

## Installing cxml

Following the build steps above, you can install the library using:

```
cmake --install .
```

This installs to the default path specified by cmake. If you want to override this, in the build step above, instead of:

```
cmake <inteface flags if any> ..
```

you can do this:
```
cmake -DCMAKE_INSTALL_PREFIX="your/preferred/path/here" <other flags> ..
```

<br/>

## Using cxml

To use/consume the library, in your project folder, create a cmake folder, and create a file in that folder called `Findcxml.cmake`.

The contents of the file should be similar to this:

```cmake
set(FIND_CXML_PATHS
    "your/path/to/the/installed/cxml/library/") # e.g. "C:/Program Files/cxml"


find_path(CXML_INCLUDE_DIR  cxml
        PATH_SUFFIXES include
        PATHS ${FIND_CXML_PATHS})


find_library(CXML_LIBRARY
        NAMES cxml
        PATH_SUFFIXES lib  # folder containing libcxml.a
        PATHS ${FIND_CXML_PATHS})
```
<br/>

**Using the Interfaces**

Depending on the way you built the library, you should provide the appropriate flags needed for your application.

For instance, if you built the library along with all the interfaces available (XPATH, SAX, QUERY), and you need to utilize all three interface in your application, then you should provide all three build flags mentioned above, in your application/project's build configuration.

Here's an example `CMakeLists.txt` that utilizes all three interfaces provided by cxml (under the assumption that the library has been built initially with all three interfaces included). This depends on the `Findcxml.cmake` file highlighted above.

```cmake
cmake_minimum_required(VERSION 3.15)
project(my_app C)

set(CMAKE_C_STANDARD 11)  # cxml requires C_STANDARD set to C11

add_executable(my_app main.c)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(FindPkgConfig)

find_package(cxml REQUIRED)

target_include_directories(my_app PUBLIC ${CXML_INCLUDE_DIR})

# we need all three interfaces, so we provide the appropriate flags
target_compile_definitions(my_app PUBLIC "CXML_USE_XPATH_MOD" "CXML_USE_QUERY_MOD" "CXML_USE_SAX_MOD")

target_link_libraries(${PROJECT_NAME} ${CXML_LIBRARY})
```
<br/>

Your project structure may look like this:

```
  my_app
  |
  └─── build
  |
  └───cmake
  |     |
  |     └─── Findcxml.cmake
  |
  └─── main.c
  |
  └─── CMakeLists.txt
```

And in `main.c`, only a single header include is sufficient for any feature needed from the library. 

```C
#include <cxml/cxml.h>

// !code me!

```

<br/>

<sup><sup>[Improve this document.](https://github.com/ziord/cxml/issues)</sup></sup>

