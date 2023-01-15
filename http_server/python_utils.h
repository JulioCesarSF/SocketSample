#pragma once

#define TO_PYTHON_GET(python_function, endpoint, callback) controller.add_get(endpoint, callback)
#define TO_PYTHON_POST(python_function, endpoint, callback) controller.add_post(endpoint, callback)
#define HOST_CONFIG(ip, port)