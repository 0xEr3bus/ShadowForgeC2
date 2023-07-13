import sqlite3


class ManageDatabase:
    def __init__(self):
        self.DB = 'TeamServer/db/database.db'
        self.connection = None

    def ConnectDatabase(self):
        try:
            self.connection = sqlite3.connect(self.DB, check_same_thread=False)
            return self.connection
        except sqlite3.Error as e:
            print(e)
            return None

    def CreateChannelTable(self):
        try:
            c = self.connection.cursor()
            ChannelsTable = """ CREATE TABLE IF NOT EXISTS Channels (
                                                id integer PRIMARY KEY,
                                                ChannelName text NOT NULL,
                                                Token text,
                                                ChannelID text
                       
                                            ); """
            c.execute(ChannelsTable)
        except sqlite3.Error as e:
            print(e)
            return None

    def AddChannel(self, ChannelName, Token, ChannelID):
        try:
            Query = '''INSERT INTO Channels(
                                            ChannelName,
                                            Token,
                                            ChannelID
                                            ) VALUES(?,?,?)'''
            cur = self.connection.cursor()
            cur.execute(Query, (ChannelName, Token, ChannelID))
            self.connection.commit()
        except sqlite3.Error as e:
            print(e)
            return None

    def DeleteChannel(self, ChannelName):
        sql = ''' DELETE FROM Channels WHERE ChannelName = ?'''
        cur = self.connection.cursor()
        cur.execute(sql, (ChannelName,))
        self.connection.commit()

    def QueryChannel(self, ChannelName):
        sql = ''' SELECT * FROM Channels WHERE ChannelName = ?'''
        cur = self.connection.cursor()
        cur.execute(sql, (ChannelName,))
        row = cur.fetchone()
        return row

    def QueryChannelByID(self, ChannelID):
        sql = ''' SELECT ChannelName FROM Channels WHERE ChannelID = ?'''
        cur = self.connection.cursor()
        cur.execute(sql, (ChannelID,))
        row = cur.fetchone()
        return row

    def QueryAllChannels(self):
        sql = ''' SELECT * FROM Channels '''
        cur = self.connection.cursor()
        cur.execute(sql)
        rows = cur.fetchall()
        return rows
