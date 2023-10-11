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
import subprocess, shlex
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
def url(video_id, wait_time, title, artist, tab):
    url = "https://www.youtube.com/watch?v=" + video_id

    # chrome_path = r"C:/Program Files/Google/Chrome/Application/chrome.exe"
    # webbrowser.register('chrome', None, webbrowser.BackgroundBrowser(chrome_path))
    # print(webbrowser.get('chrome').open_new_tab(url))
    send = title + ',' + artist + ',' + str(wait_time)
    print(send)
    # while True:
    print("waiting",ser.in_waiting)
    ser.readline()
    ser.write((send).encode('Shift-JIS'))
        # time.sleep(1)
        # if ser.in_waiting:
            # data = ser.readline().decode('utf-8').strip()
            # print("url",data)
            # break
        
    # ser.write((title).encode('Shift-JIS'))
    # ser.write((artist).encode('Shift-JIS'))
    # ser.write((wait_time).encode('Shift-JIS'))
    # or
    # ser.write(title.encode('Shift-JIS'))
    # ser.write(artist.encode('Shift-JIS'))
    if tab == 0:
        webbrowser.open(url, new=1, autoraise=True)
    else:
        webbrowser.open(url, new=2, autoraise=True)
    # print("title: "+title+", artist: "+artist)
    time.sleep(wait_time)

# csv形式の文字列をint型配列に変換
def csv_to_int_list(csv_string):
    """
    Convert a CSV formatted string to a list of integers.
    
    Parameters:
    - csv_string: String formatted as CSV.
    
    Returns:
    - List of integers.
    """
    return [int(x) for x in csv_string.split(',')]

#list
def makelist(data_list,listnum,settime,neginegi_time,nnum):
    i=nnum
    flag=0
    if listnum==nnum:
        return 0
    for i in range(nnum,listnum):
        if(i==listnum):
            return 0
        print('time:',neginegi_time,'nnum:',nnum,'i:',i)
        if flag==0:
            flag=1
            continue
        alt=data_list[i][7]
        alt = float(alt)
        alt = int(alt)
        tmptime=neginegi_time+alt
        print(tmptime,'=',neginegi_time,'+',alt)
        if tmptime>settime:
            i+=1
            continue
        else:
            if settime-tmptime<10:
                print(i)
                while True:
                    print(str(tmptime))
                    print("waiting",ser.in_waiting)
                    ser.readline()
                    ser.write((str(tmptime)).encode('Shift-JIS'))
                    time.sleep(1)
                    if ser.in_waiting:
                        data = ser.readline().decode('utf-8').strip()
                        print("time",data)
                        break
                print('out')
                url(data_list[i][6],alt,data_list[i][2],data_list[i][5],0)
                return 1

        print('call makelist',tmptime,i)
        res=makelist(data_list,listnum,settime,tmptime,i)
        if res==1:
            url(data_list[i][6],alt,data_list[i][2],data_list[i][5],1)
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
            
        if(len(kana) > 48):
            kana = kana[:45]
            kana += "..."

        if(len(roma) > 48):
            roma = roma[:45]
            roma += "..."
            
        art_kana = ""
        art_roma = ""
        for kanji in item['snippet']['channelTitle']:
            result = kks.convert(kanji)
            art_kana += result[0]['kana']
            art_roma += result[0]['passport']
            
        if(len(kana) > 48):
            kana = kana[:45]
            kana += "..."

        if(len(roma) > 48):
            roma = roma[:45]
            roma += "..."

        ws.writerow([item['snippet']['title'],kana,roma,item['snippet']['channelTitle'],art_kana,art_roma,item['id'],pt2sec(pt_time)])

#================================================================================
# 配列を指定した個数ごとに分割
#================================================================================
def split_list(l, n):
    for idx in range(0, len(l), n):
        yield l[idx:idx + n]

# args = shlex.split("sudo dhclient wlan0")
# ret = subprocess.call(args)
            
# YoutubeAPI用キー
API_KEY = 'AIzaSyDVqEeWLn8aQv8-y3lZ3EKrtD7k9TTg2dQ' #ここに各々で取得したYoutube用のAPIキーを入れる
FILENAME = ["Youtube1.csv", "Youtube2.csv", "Youtube3.csv"]
youtube = initYoutube(API_KEY)

#「ワークシート名:プレイリストID」の辞書型配列にしておく
# playList = [{
#     'cc3.0':'PLhsPoYH9s2V0U8TkF8hqz_uVbbII1tjAv',
#     'cc3.0追加':'PL4EJWCM_RXBHad4pVU1pNYFNXTAgy_crx',
# }, {
#     # 'ボカロ':'PLVe76sDJ5THhs3NkKPjxlNFWxkcJ1YTwa',
#     'ボカロ':'PLO3JxJH4SmvPpuLRqtwr5aWuie9GVTfaj',
# }, {
#     '趣味':'PLMzfDtnmpEYS2gBPKHfZB2QX0yYhMb4Px'
# }]
playList = ["PL4EJWCM_RXBHad4pVU1pNYFNXTAgy_crx", "PLO3JxJH4SmvPpuLRqtwr5aWuie9GVTfaj", "PLMzfDtnmpEYS2gBPKHfZB2QX0yYhMb4Px"]

dt_now = datetime.datetime.now()
ser = serial.Serial('/dev/ttyUSB0', 115200)  # Adjust the device and baud rate accordingly
tab = 0

while True:
    if ser.in_waiting:
        data = ser.readline().decode('utf-8').strip()
        print(f"Received: {data}")
        if 'sendCode:' in data:
            code = data[data.find(":")+1 : ]
            #print(code)
            print(csv_to_int_list(code))
            code = csv_to_int_list(code)
            ser.write(('get\n').encode('UTF-8'))
            break
        
file_num = code[1]
file_num = int(file_num)
print('FILE num: ',file_num)

settime = code[2]
settime = int(settime)
settime = settime * 60
print('Set time: ',settime)

with open(FILENAME[file_num-11], 'w', encoding='Shift-JIS', errors='ignore', newline='') as f:
    writer = csv.writer(f)
    setCountDetail(writer,getIdListFromPlaylist(playList[file_num-11],youtube),youtube)    

print('start serch')
i=0
listmax=500
result=0
flag=0

with open(FILENAME[file_num-11],"r",encoding="Shift-JIS") as f:
    tester=csv.reader(f)
    row_list = [row for row in tester]

for i in range(len(row_list)):
    if flag == 0:
        flag = 1
        continue
    alt = row_list[i][7]
    alt = float(alt)
    print('alt=',alt,'i=',i)
    result=makelist(row_list,len(row_list),settime,int(alt),i)
    if result==1:
        print(i)
        url(row_list[i][6],alt,row_list[i][2],row_list[i][5],1)
        print('break')
        break
if result==0:
    print('cannot')
else:
    ser.readline()
    print("waiting",ser.in_waiting)
    ser.write(b'Listening_completed.')
    print("End")
    while True:
        print("waiting",ser.in_waiting)
        if ser.in_waiting:
            print("End")
            # ser.readline()
            data = ser.readline().decode('utf-8').strip()
            print(data)
            if data == 'Shutdown':
                os.system("sudo shutdown -h now")
