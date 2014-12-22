/*
 * This file is part of VRController.
 * Copyright (c) 2014 Fabien Caylus <toutjuste13@gmail.com>
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <vector>
#include <string>
#include <cstdio>
#include <cstdint>
#include <thread>
#include <functional>
#include <cerrno>

#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

#define DEFAULT_RFCOMM_CHANNEL 22
#define AUTO_RFCOMM_CHANNEL 0

//
// Single Header that manage all bluetooth operations as a server
// Only work in linux (use of BlueZ library)
// This class use c++11 features
class BluetoothManager
{
    public:
        // Enumeration that describes the state of the manager
        // Each state is usually completed in the order as they are defined
        enum class State
        {
            // Default state
            NO_STATE = 0,

            CONNECTED_TO_SOCKET = 1,
            BOUND_TO_SOCKET = 2,
            LISTENING = 3,
            CONNECTED_TO_CLIENT = 4
        };

        // Enumeration that specify the error
        // After each method call, you must call the method hasError() to check if an error occurs
        // or create a handler to know when there is an error.
        enum class Error
        {
            NO_ERROR = 0,

            REGISTER_SDP_SERVICE = 1,
            SOCKET_CONNECTION = 2,
            SOCKET_BIND = 3,
            START_LISTEN = 4,
            CLIENT_CONNECTION = 5,

            SEND_MSG = 50,
            CLOSE_SDP_SERVICE = 51,

            // Errors for not correct state
            NOT_IN_BOUND_STATE = 100,
            NOT_IN_CONNECTED_STATE = 101,
            // RFCOMM channel not in range 0-30 (0 means for auto)
            BAD_RFCOMM_CHANNEL = 200
        };

    protected:
        // Represent any BDADDR address
        // This is a workaround for the C macro BDADDR_ANY
        // since it doesn't work in C++ (with -fpermissive)
        bdaddr_t BDADDRAny = {{0, 0, 0, 0, 0, 0}};
        bdaddr_t BDADDRLocal = {{0, 0, 0, 0xff, 0xff, 0xff}};
        bdaddr_t BDADDRAll = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

        // Network socket
        int _socket = -1;
        // Represent the connected client
        int _client = -1;

        // RFCOMM Channel used for the connection
        int _RFCOMMChannel;

        // Contains information about the local socket address
        struct sockaddr_rc _localSockAddr;
        // Contains information about the remote socket address
        struct sockaddr_rc _remoteSockAddr;
        socklen_t _remoteSockLength = sizeof(_remoteSockAddr);

        // Represent the session with the SDP service
        // Create by registerSDPService() method
        sdp_session_t *_sdpSession = nullptr;

        // Current state of the manager
        State _state = State::NO_STATE;
        // Contains all error occured
        std::vector<Error> _errors;

        // Accept thread
        std::thread _acceptThread;

        // This handler is called every time the state change.
        // The State argument is the new state of the manager.
        std::function<void(State)> _stateChangedHandler;

        // Handler called every time a new error happens.
        // The new error is passed to the handler
        std::function<void(Error)> _newErrorHandler;

        // Contains the UUID used for SDP service
        // Don't change this since it's the same UUID for all games that used this controller
        const std::uint32_t UUID[4] = { 0x0123, 0x4567, 0x89AB, 0xCDEF };

        //
        // Protected methods
        //

        // This method is used to change the state, and will call the handler
        // stored in _stateChangedHandler
        void setState(State state)
        {
            _state = state;
            // Call the handler
            _stateChangedHandler(_state);
        }

        void appendError(Error error)
        {
            _errors.push_back(error);
            _newErrorHandler(error);
        }

        // Register a SDP service to enable client to connect to this server
        // Return the error code
        int registerSDPService()
        {
            // Register all informations and UUIDs
            uuid_t rootUUID, l2capUUID, rfcommUUID, serviceUUID;
            sdp_list_t *l2capList = 0,
                       *rfcommList = 0,
                       *rootList = 0,
                       *protoList = 0,
                       *accessProtocolList = 0;
            sdp_data_t *channel = 0;

            // Contains infos of the SDP service
            sdp_record_t *record = sdp_record_alloc();

            // Set the general service UUID
            sdp_uuid128_create(&serviceUUID, &UUID);
            sdp_set_service_id(record, serviceUUID);

            // Make the service record publicly browsable
            sdp_uuid16_create(&rootUUID, PUBLIC_BROWSE_GROUP);
            rootList = sdp_list_append(0, &rootUUID);
            sdp_set_browse_groups(record, rootList);

            // Set L2CAP information
            sdp_uuid16_create(&l2capUUID, L2CAP_UUID);
            l2capList = sdp_list_append(0, &l2capUUID);
            protoList = sdp_list_append(0, l2capList);

            // Set RFCOMM information
            sdp_uuid16_create(&rfcommUUID, RFCOMM_UUID);
            channel = sdp_data_alloc(SDP_UINT8, &_RFCOMMChannel);
            rfcommList = sdp_list_append(0, &rfcommUUID);
            sdp_list_append(rfcommList, channel);
            sdp_list_append(protoList, rfcommList);

            // Attach protocol information to service record
            accessProtocolList = sdp_list_append(0, protoList);
            sdp_set_access_protos(record, accessProtocolList);

            // Re-init the session
            if(_sdpSession != nullptr)
            {
                delete _sdpSession;
                _sdpSession = nullptr;
            }

            _sdpSession = 0;

            // Connect to the local SDP server, register the service record
            _sdpSession = sdp_connect(&BDADDRAny, &BDADDRLocal, SDP_RETRY_IF_BUSY);
            const int errorCode = sdp_record_register(_sdpSession, record, 0);

            // Cleanup all lists
            sdp_data_free(channel);
            sdp_list_free(l2capList, 0);
            sdp_list_free(rfcommList, 0);
            sdp_list_free(rootList, 0);
            sdp_list_free(protoList, 0);
            sdp_list_free(accessProtocolList, 0);
            sdp_record_free(record);

            // Return 0 on success and -1 on error
            return errorCode;
        }

    public:

        // Default constructor
        // If the RFCOMM channel is set to 0, it will be auto generated by the manager
        BluetoothManager(int rfcommChannel = DEFAULT_RFCOMM_CHANNEL,
                         std::function<void(State)> stateChangedHandler = [](State newState){ (void)newState; },
                         std::function<void(Error)> newErrorHandler = [](Error newError){ (void)newError; })
        {
            errno = 0;
            // Set handlers
            setStateChangedHandler(stateChangedHandler);
            setNewErrorHandler(newErrorHandler);

            // Call the handler first time in the constructor
            _stateChangedHandler(_state);
            _newErrorHandler(Error::NO_ERROR);

            if(!setRFCOMMChannel(rfcommChannel))
                return;

            // Create the network socket
            _socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
            if(_socket < 0)
                appendError(Error::SOCKET_CONNECTION);
            else
            {
                setState(State::CONNECTED_TO_SOCKET);

                // Bind the network socket to the corresponding RFCOMM channel
                _localSockAddr.rc_family = AF_BLUETOOTH;
                _localSockAddr.rc_bdaddr = BDADDRAny;
                _localSockAddr.rc_channel = (std::uint8_t) _RFCOMMChannel;

                const int bindReturnValue = bind(_socket, (struct sockaddr *)&_localSockAddr, sizeof(_localSockAddr));

                // Check the return value of bind() function
                if(bindReturnValue < 0)
                    appendError(Error::SOCKET_BIND);
                else
                    setState(State::BOUND_TO_SOCKET);
            }
        }

        // Default destructor
        // Shutdown all opened sockets
        ~BluetoothManager()
        {
            errno = 0;
            if(_socket >= 0)
                shutdown(_socket, SHUT_RDWR);
            if(_client >= 0)
                shutdown(_client, SHUT_RDWR);

            // Join the accept thread here to avoid memory leaks
            _acceptThread.join();
        }

        // Get the current state
        State state() const
        {
            return _state;
        }

        // Check if there is errors
        bool hasError() const
        {
            return !_errors.empty();
        }

        // Retrieve the last error and remove it from the list
        Error retrieveLastError()
        {
            if(_errors.empty())
                return Error::NO_ERROR;

            Error er = _errors.back();
            _errors.pop_back();
            return er;
        }

        // Retrieve the first error and remove it from the list
        Error retrieveFirstError()
        {
            if(_errors.empty())
                return Error::NO_ERROR;
            Error er = _errors.front();
            _errors.erase(_errors.begin());
            return er;
        }

        void setStateChangedHandler(std::function<void(State)> handler)
        {
            _stateChangedHandler = handler;
        }

        void setNewErrorHandler(std::function<void(Error)> handler)
        {
            _newErrorHandler = handler;
        }

        // The channel must be in range 0-30
        // If the channel equals 0, it will be auto generated
        bool setRFCOMMChannel(int channel)
        {
            if(channel >= 0 && channel < 31)
            {
                _RFCOMMChannel = channel;
                return true;
            }

            appendError(Error::BAD_RFCOMM_CHANNEL);
            return false;
        }

        int rfcommChannel() const
        {
            return _RFCOMMChannel;
        }

        // Start to listen for incoming connections
        void startListening()
        {
            errno = 0;
            // This function need the socket to be bind
            if(_state != State::BOUND_TO_SOCKET)
            {
                appendError(Error::NOT_IN_BOUND_STATE);
                return;
            }

            if(listen(_socket, 1) < 0)
                appendError(Error::START_LISTEN);
            else
            {
                setState(State::LISTENING);

                // Get the real channel if set to 0 (auto-generated to the first available)
                if(_RFCOMMChannel == 0)
                {
                    struct sockaddr_rc localAddr;
                    socklen_t addrLength = sizeof(localAddr);
                    getsockname(_socket, (struct sockaddr *)&localAddr, &addrLength);

                    _RFCOMMChannel = localAddr.rc_channel;
                }

                // Register the SDP service here (with the correct channel)
                if(registerSDPService() < 0)
                {
                    appendError(Error::REGISTER_SDP_SERVICE);
                    return;
                }

                // Accept one connection in an another thread
                _acceptThread = std::thread(&BluetoothManager::acceptConnection, this, _socket, (struct sockaddr *)&_remoteSockAddr, &_remoteSockLength, [this](int clientID) {
                    _client = clientID;

                    if(_client < 0)
                        appendError(Error::CLIENT_CONNECTION);
                    else
                    {
                        setState(State::CONNECTED_TO_CLIENT);

                        // Close the SDP service when connected
                        sdp_close(_sdpSession);
                    }
                });
            }
        }

        // This function is usually called in the startListening method.
        // This function is blocking, must be called in a different thread.
        // The function param is used to notify when the accept operation is finished and
        // the int param of this function represent the new client ID.
        void acceptConnection(int socket, __SOCKADDR_ARG remoteAddr, socklen_t *__restrict remoteAddrLength, std::function<void(int)> onResultFunction)
        {
            errno = 0;
            onResultFunction(accept(socket, remoteAddr, remoteAddrLength));
        }

        // Return the current client address
        const std::string clientAddress()
        {
            return addressFromBDADDR(&_remoteSockAddr.rc_bdaddr);
        }

        // Return the channel on the client is connected
        std::uint8_t clientChannel()
        {
            return _remoteSockAddr.rc_channel;
        }

        // Send a message to the connected client
        // Usually, prefer other variants with more friendly types
        void sendMessage(const void *message, size_t msgSize)
        {
            errno = 0;
            if(_state != State::CONNECTED_TO_CLIENT)
            {
                appendError(Error::NOT_IN_CONNECTED_STATE);
                return;
            }

            if(send(_client, message, msgSize, MSG_DONTWAIT) < 0)
                appendError(Error::SEND_MSG);
        }

        void sendMessage(const std::string message)
        {
            sendMessage(message.data(), message.size());
        }

        void sendMessage(const std::uint8_t number)
        {
            sendMessage(&number, 1);
        }

        std::string serviceUUID() const
        {
            uuid_t returnUUID;
            sdp_uuid128_create(&returnUUID, &UUID);
            return uuidToString(&returnUUID);
        }

        //
        // Static methods
        //

        static std::string addressFromBDADDR(const bdaddr_t *ba)
        {
            char buffer[1024] = {0};
            ba2str(ba, buffer);
            return std::string(buffer);
        }

        static std::string uuidToString(const uuid_t *uuid)
        {
            char buffer[1024] = {0};
            sdp_uuid2strn(uuid, buffer, 1024);
            return std::string(buffer);
        }

        static std::string errorString(Error error)
        {
            switch (error) {
                case Error::NO_ERROR:
                    return "NO_ERROR";
                    break;
                case Error::REGISTER_SDP_SERVICE:
                    return "REGISTER_SDP_SERVICE";
                    break;
                case Error::SOCKET_CONNECTION:
                    return "SOCKET_CONNECTION";
                    break;
                case Error::SOCKET_BIND:
                    return "SOCKET_BIND";
                    break;
                case Error::START_LISTEN:
                    return "START_LISTEN";
                    break;
                case Error::CLIENT_CONNECTION:
                    return "CLIENT_CONNECTION";
                    break;
                case Error::SEND_MSG:
                    return "SEND_MSG";
                    break;
                case Error::CLOSE_SDP_SERVICE:
                    return "CLOSE_SDP_SERVICE";
                    break;
                case Error::NOT_IN_BOUND_STATE:
                    return "NOT_IN_BOUND_STATE";
                    break;
                case Error::NOT_IN_CONNECTED_STATE:
                    return "NOT_IN_CONNECTED_STATE";
                    break;
                case Error::BAD_RFCOMM_CHANNEL:
                    return "BAD_RFCOMM_CHANNEL";
                    break;
                default:
                    return std::to_string(static_cast<int>(error));
                    break;
            }
        }

        static std::string stateString(State state)
        {
            switch (state) {
                case State::NO_STATE:
                    return "NO_STATE";
                    break;
                case State::CONNECTED_TO_SOCKET:
                    return "CONNECTED_TO_SOCKET";
                    break;
                case State::BOUND_TO_SOCKET:
                    return "BOUND_TO_SOCKET";
                    break;
                case State::LISTENING:
                    return "LISTENING";
                    break;
                case State::CONNECTED_TO_CLIENT:
                    return "CONNECTED_TO_CLIENT";
                    break;
                default:
                    return std::to_string(static_cast<int>(state));
                    break;
            }
        }
};

#endif // BLUETOOTHMANAGER_H
