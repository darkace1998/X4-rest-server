#pragma once

#include "../httpserver/HttpServer.h"
#include "../ffi/FFIInvoke.h"

#include "../_lua_.h"


inline void RegisterPlayerFunctions(INIT_PARAMS()) {

    HttpServer::AddEndpoint(SIMPLE_GET_HANDLER(GetPlayerComputerID));
    HttpServer::AddEndpoint(SIMPLE_GET_HANDLER(GetPlayerContainerID));
    HttpServer::AddEndpoint(SIMPLE_GET_HANDLER(GetPlayerControlledShipID));
    HttpServer::AddEndpoint(SIMPLE_GET_HANDLER(GetPlayerID));
    HttpServer::AddEndpoint(SIMPLE_GET_HANDLER(GetPlayerObjectID));
    HttpServer::AddEndpoint(SIMPLE_GET_HANDLER(GetPlayerOccupiedShipID));

    HttpServer::AddEndpoint(SIMPLE_GET_HANDLER(GetCreditsDueFromPlayerBuilds));
    HttpServer::AddEndpoint(SIMPLE_GET_HANDLER(GetCreditsDueFromPlayerTrades));

    HttpServer::AddEndpoint(SIMPLE_GET_HANDLER(GetPlayerName));
    HttpServer::AddEndpoint({"/GetPlayerFactionName", HttpServer::Method::GET,
        [ & ](const httplib::Request& req, httplib::Response& res) {
            bool userawname = false;
            const auto userawnameParam = req.get_param_value("userawname");
            if (userawnameParam == "1" || userawnameParam == "true") {
                userawname = true;
            }

            const auto callResult = invoke(GetPlayerFactionName, userawname);
            SET_CONTENT((callResult));
        }});

    HttpServer::AddEndpoint(SIMPLE_GET_HANDLER(GetPlayerZoneID));

    HttpServer::AddEndpoint({"/GetStats", HttpServer::Method::GET,
        [ & ](const httplib::Request& req, httplib::Response& res) {
            bool include_hidden = false;

            try {
                const auto hidden_param = req.get_param_value("hidden");
                include_hidden = hidden_param == "true" || hidden_param == "1";
            }
            catch (...) {
                // ignore
            }

            if (ui_lua_state != nullptr) {
                const auto get_stats_lua =
                    R"(    
local statTable = {}
local stats = GetAllStatIDs()
for i = 1, #stats do
	local hidden, displayname = GetStatData(stats[i], "hidden", "displayname")
	if not hidden then
		statTable[stats[i]] = GetStatData(stats[i], "displayvalue")
	else 
	   	)" + std::string{include_hidden ? "" : "-- "} +
                    R"(statTable["hidden:" .. stats[i]] = GetStatData(stats[i], "displayvalue")
	end
end

return json.encode(statTable)
                )";
                const auto callResult = executeLua(get_stats_lua, true, true);
                SET_CONTENT((callResult));
                return;
            }
            SET_CONTENT(({}));
        }});
}