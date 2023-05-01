/*
 * http_api.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-4-12

Description: Main Http API

**************************************************/

#include "openapt.hpp"
#include "http_api.hpp"

namespace OpenAPT
{
    void init_handler(crow::SimpleApp &app)
    {
        CROW_ROUTE(app, "/")
        ([]
        { return crow::mustache::load("index.html").render(); });

        CROW_ROUTE(app, "/client")
        ([]
        { return crow::mustache::load("client.html").render(); });

         // Route to handle login requests
        CROW_ROUTE(app, "/login")
            .methods("POST"_method)([](const crow::request& req) -> crow::response {
                try {
                    // Parse JSON data from the request body
                    json reqJson = json::parse(req.body);

                    // Extract username and password from the request JSON
                    std::string username = reqJson["username"].get<std::string>();
                    std::string password = reqJson["password"].get<std::string>();

                    if (username.empty()) {
                        throw std::invalid_argument("'username' parameter is required");
                    }

                    if (password.empty()) {
                        throw std::invalid_argument("'password' parameter is required");
                    }

                    // Load the password JSON file
                    std::ifstream file("password.json");
                    json passwordJson;
                    if (file.good()) {
                        file >> passwordJson;
                    }
                    file.close();

                    // Check if the user exists and the password matches
                    if (!passwordJson[username].is_null() && password == passwordJson[username].get<std::string>()) {
                        // Set user session data to allow access to other pages
                        // Redirect to the client page
                        return crow::response{ 302, "<html><head><meta http-equiv='refresh' content='0; url=/client'></head></html>" };
                    } else {
                        // Return an error response if username or password is incorrect
                        json resJson;
                        resJson["error"] = "Invalid username or password";
                        return crow::response{ 400, resJson.dump() };
                    }
                } catch (const std::exception& e) {
                    // Handle exceptions and return an error response
                    json resJson;
                    resJson["error"] = e.what();
                    return crow::response{ 400, resJson.dump() };
                }
            });

        CROW_ROUTE(app, "/greeting")
            .methods("GET"_method)([](const crow::request &req)
                                {
                try {
                    std::string name = req.url_params.get("name");

                    // Do some processing on the parameter
                    if (name.empty()) {
                        throw std::invalid_argument("'name' parameter is required");
                    }

                    // Construct a response with the greeting
                    nlohmann::json resJson;
                    resJson["message"] = "Hello, " + name + "!";
                    return crow::response{ resJson.dump() };
                } catch (const std::exception& e) {
                    // Handle exceptions and return an error response
                    nlohmann::json resJson;
                    resJson["error"] = e.what();
                    return crow::response{ 400, resJson.dump() };
                }
            });

        // Route to accept GET and POST requests and return a greeting based on JSON data
        CROW_ROUTE(app, "/json")
            .methods("GET"_method, "POST"_method)([](const crow::request &req)
                                                {
                if (req.method == crow::HTTPMethod::Get) {
                    // Handle GET request
                    return crow::response{ "This is a GET request" };
                } else if (req.method == crow::HTTPMethod::Post) {
                    // Parse the JSON data from the request body
                    nlohmann::json reqJson = nlohmann::json::parse(req.body);

                    // Do some processing on the JSON data
                    nlohmann::json resJson;
                    resJson["message"] = "Hello, " + reqJson["name"].get<std::string>() + "!";

                    // Return the JSON data in the response body
                    return crow::response{ resJson.dump() };
                } else {
                    // Handle other HTTP methods
                    return crow::response(405);
                }
            });
    }

} // namespace OpenAPT
