from googleapiclient.discovery import build
import json
import openpyxl
import re
import datetime
import csv
import pprint
import pykakasi
import webbrowser
import time
import os
import random
import serial
from datetime import date, timedelta, timezone
from openpyxl.styles import Font
#================================================================================
# 初期化
#================================================================================
def initYoutube(API_KEY):
    API_SERVICE_NAME = "youtube"
    API_VERSION = "v3"
    return build(API_SERVICE_NAME, API_VERSION, developerKey=API_KEY)

def pt2sec(pt_time):
    '''ISO表記の動画時間を秒に変換 '''
    pttn_time = re.compile(r'PT(\d+H)?(\d+M)?(\d+S)?')
    keys = ['hours', 'minutes', 'seconds']
    m = pttn_time.search(pt_time)
    if m:
        kwargs = {k: 0 if v is None else int(v[:-1])
                    for k, v in zip(keys, m.groups())}
        return timedelta(**kwargs).total_seconds()
    else:
        msg = '{} is not valid ISO time format.'.format(pt_time)
        raise ValueError(msg)

# 再生用
def url(video_id, wait_time, title, artist):
    url = "https://www.youtube.com/watch?v=" + video_id

    chrome_path = r"C:/Program Files/Google/Chrome/Application/chrome.exe"
    webbrowser.register('chrome', None, webbrowser.BackgroundBrowser(chrome_path))
    print(webbrowser.get('chrome').open_new_tab(url))
    ser.write((title+'\n').encode('Shift-JIS'))
    ser.write((artist+'\n').encode('Shift-JIS'))
    # or
    # ser.write(title.encode('Shift-JIS'))
    # ser.write(artist.encode('Shift-JIS'))
    # webbrowser.open(url, new=1, autoraise=True)
    # print("title: "+title+", artist: "+artist)
    time.sleep(wait_time)

def makelist(data_list,listnum,settime,time,nnum):
    i=nnum
    flag=0
    if listnum==nnum:
        return 0
    for i in range(nnum,listnum):
        if(i==listnum):
            return 0
        print('time:',time,'nnum:',nnum,'i:',i)
        if flag==0:
            flag=1
            continue
        alt=data_list[i][7]
        alt = float(alt)
        alt = int(alt)
        tmptime=time+alt
        print(tmptime,'=',time,'+',alt)
        if tmptime>settime:
            i+=1
            continue
        else:
            if settime-tmptime<1:
                print(i)
                ser.write((tmptime+'\n').encode('utf-8'))
                url(data_list[i][6],alt,data_list[i][2],data_list[i][5])
                return 1

        print('call makelist',tmptime,i)
        res=makelist(data_list,listnum,settime,tmptime,i)
        if res==1:
            url(data_list[i][6],alt,data_list[i][2],data_list[i][5])
            return 1
        
    print('kill by [i]:',i)
    return 0

#================================================================================
# プレイリストIDを渡して動画IDリストを得る
#================================================================================
def getIdListFromPlaylist(id_,youtube):

    nextPageToken = 'start'
    response = []

    while(nextPageToken is not None):

        if(nextPageToken == 'start'):
            search_response = youtube.playlistItems().list(
            part= 'snippet, contentDetails',
            playlistId=id_,
            maxResults = 50,
            ).execute()
        else:
            search_response = youtube.playlistItems().list(
            part= 'snippet, contentDetails',
            playlistId=id_,
            maxResults = 50,
            pageToken = nextPageToken
            ).execute()

        if('nextPageToken' in search_response):
            nextPageToken = search_response['nextPageToken']
        else:
            nextPageToken = None
        
        for item in search_response['items']:
            response.append(item['snippet']['resourceId']['videoId'])

    response.reverse()   
    return response

#================================================================================
# YoutubeのAPIを叩いて統計情報を取得する
#================================================================================
def getCountDetails(id_, youtube):

    #50件ずつに分割
    idLists = split_list(id_,50)
    response = []

    for idList in idLists:
        search_response = youtube.videos().list(
        part= 'statistics,snippet, contentDetails',
        id=idList,
        ).execute()

        response.extend(search_response['items'])

    return response

#================================================================================
# 指定ワークシートからIDリストを取得して数値を更新する
#================================================================================
def setCountDetail(ws,idList,youtube):

    result = getCountDetails(idList,youtube)

    # ws.writerow(['曲名','タイトル','title','投稿者','アーティスト','artist','video_id','再生時間'])

    kks = pykakasi.kakasi()

    for item in result:

        '''動画時間を抜き出す（ISO表記を秒に変換）'''
        content_details = item['contentDetails']
        pt_time = content_details['duration']

        kana = ""
        roma = ""
        for kanji in item['snippet']['title']:
            result = kks.convert(kanji)
            kana += result[0]['kana']
            roma += result[0]['passport']
        
        # if(len(kana) > 15):
        #     kana = kana[:12]
        #     kana += "..."

        # if(len(roma) > 15):
        #     roma = roma[:12]
        #     roma += "..."

        art_kana = ""
        art_roma = ""
        for kanji in item['snippet']['channelTitle']:
            result = kks.convert(kanji)
            art_kana += result[0]['kana']
            art_roma += result[0]['passport']
        
        # if(len(art_kana) > 15):
        #     art_kana = art_kana[:12]
        #     art_kana += "..."

        # if(len(art_roma) > 15):
        #     art_roma = art_roma[:12]
        #     art_roma += "..."

        ws.writerow([item['snippet']['title'],kana,roma,item['snippet']['channelTitle'],art_kana,art_roma,item['id'],pt2sec(pt_time)])

#================================================================================
# 配列を指定した個数ごとに分割
#================================================================================
def split_list(l, n):
    for idx in range(0, len(l), n):
        yield l[idx:idx + n]

# YoutubeAPI用キー
API_KEY = 'AIzaSyDVqEeWLn8aQv8-y3lZ3EKrtD7k9TTg2dQ' #ここに各々で取得したYoutube用のAPIキーを入れる
FILENAME = ["Youtube1.csv", "Yotube2.csv", "Youtube3.csv"]
youtube = initYoutube(API_KEY)

#「ワークシート名:プレイリストID」の辞書型配列にしておく
playList = {
'cc3.0':'PLhsPoYH9s2V0U8TkF8hqz_uVbbII1tjAv',
'ボカロ':'PLVe76sDJ5THhs3NkKPjxlNFWxkcJ1YTwa',
}

dt_now = datetime.datetime.now()
ser = serial.Serial('/dev/ttyUSB0', 115200)  # Adjust the device and baud rate accordingly

file_num = 1
# file_num = ser.readline().decode('utf-8').strip()
# file_num = int(file_num)
#print('FILE num:%d',file_num)

with open(FILENAME[file_num-1], 'w', encoding='Shift-JIS', errors='ignore', newline='') as f:
    writer = csv.writer(f)
    for key in playList:
        setCountDetail(writer,getIdListFromPlaylist(playList[key],youtube),youtube)    

print('start serch')
i=0
listmax=500
result=0
flag=0

with open(FILENAME[file_num-1],"r",encoding="Shift-JIS") as f:
    tester=csv.reader(f)
    row_list = [row for row in tester]

settime = 300
# settime = ser.readline().decode('utf-8').strip()
# settime = int(settime)

for i in range(len(row_list)):
    if flag==0:
        flag=1
        continue
    #print(row[1])
    alt = row_list[i][7]
    alt = float(alt)
    print('alt=',alt,'i=',i)
    result=makelist(row_list,len(row_list),settime,int(alt),i)
    if result==1:
        print(i)
        print('break')
        break
if result==0:
    print('cannot')
else:
    ser.write(b'Listening_completed.\n')
    print("End")
