#pragma once

#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <climits>
#include <deque>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <typeinfo>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>

#ifdef __linux__
#include <sys/epoll.h>
#else
#include <sys/event.h>
#endif
