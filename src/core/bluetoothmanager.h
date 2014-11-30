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
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>

#define DEFAULT_RFCOMM_CHANNEL 22

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
            CONNECTED_TO_CLIENT = 4,
        };

        // Enumeration that specify the error
        // After each method call, you must call the method hasError() to check if an error occurs
        // or create a handler to know when there is an error.
        enum class Error
        {
            NO_ERROR = 0,

            SOCKET_CONNECTION = 1,
            SOCKET_BIND = 2,
            START_LISTEN = 3,
            CLIENT_CONNECTION = 4,
            SEND_MSG = 5,

            // Errors for not correct state
            NOT_IN_BOUND_STATE = 100,
            NOT_IN_CONNECTED_STATE = 101,
            // RFCOMM Channel not in range 1-30
            BAD_RFCOMM_CHANNEL = 200
        };

    private:
        // Network socket
        int _socket;

        // RFCOMM Channel used for the connection
        int _RFCOMMChannel;

        // Contains information about the local socket address
        struct sockaddr_rc _localSockAddr;
        // Contains information about the remote socket address
        struct sockaddr_rc _remoteSockAddr;
        socklen_t _remoteSockLength = sizeof(_remoteSockAddr);

        // Represent the connected client
        int _client;

        // Current state of the manager
        State _state = State::NO_STATE;
        // Contains all error occured
        std::vector<Error> _errors;

        // Represent any BDADDR address
        // This is a workaround for the C macro BDADDR_ANY
        // since it doesn't work in C++ (with -fpermissive)
        bdaddr_t _bdaddrAny = {{0, 0, 0, 0, 0, 0}};

        // Accept thread
        std::thread _acceptThread;

        // This handler is called every time the state change.
        // The State argument is the new state of the manager.
        std::function<void(State)> _stateChangedHandler;

        // Handler called every time a new error happens.
        // The new error is passed to the handler
        std::function<void(Error)> _newErrorHandler;

        //
        // Private method
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

    public:

        // Default constructor
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

                // Bind the network socket to the default port
                // local bluetooth adapter
                _localSockAddr.rc_family = AF_BLUETOOTH;
                _localSockAddr.rc_bdaddr = _bdaddrAny;
                _localSockAddr.rc_channel = (uint8_t) _RFCOMMChannel;

                const int bindReturn = bind(_socket, (struct sockaddr *)&_localSockAddr, sizeof(_localSockAddr));
                if(bindReturn < 0)
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
            shutdown(_client, SHUT_RDWR);
            shutdown(_socket, SHUT_RDWR);

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

        // The channel must be in range 1-30
        bool setRFCOMMChannel(int channel)
        {
            if(channel > 0 && channel < 31)
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

                // Accept one connection in an another thread
                _acceptThread = std::thread(&BluetoothManager::acceptConnection, this, _socket, (struct sockaddr *)&_remoteSockAddr, &_remoteSockLength, [this](int clientID) {
                    _client = clientID;

                    if(_client < 0)
                        appendError(Error::CLIENT_CONNECTION);
                    else
                        setState(State::CONNECTED_TO_CLIENT);
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

        //
        // Static methods
        //

        static std::string addressFromBDADDR(const bdaddr_t *ba)
        {
            char buffer[1024] = {0};
            ba2str(ba, buffer);
            return std::string(buffer);
        }

        static std::string errorString(Error error)
        {
            switch (error) {
                case Error::NO_ERROR:
                    return "NO_ERROR";
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
