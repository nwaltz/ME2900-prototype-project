import spotipy
from spotipy.oauth2 import SpotifyClientCredentials
from spotipy.oauth2 import SpotifyOAuth
import requests
import cred
import time
import serial

scope = "user-read-playback-state user-modify-playback-state "

arduino_out = serial.Serial(port = 'COM5', baudrate = 9600, timeout = .01)
#arduino_in = serial.Serial(port = 'COM5', baudrate = 9600, timeout = .01)
time.sleep(3)

sp_oauth = spotipy.oauth2.SpotifyOAuth(
    client_id=cred.client_id, 
    client_secret=cred.client_secret, 
    redirect_uri = cred.redirect_uri,
    scope=scope
    )
sp = spotipy.Spotify(auth_manager=SpotifyOAuth(
    client_id=cred.client_id, 
    client_secret=cred.client_secret, 
    redirect_uri = cred.redirect_uri,
    scope=scope
    ))

last_song = ' '
last_artist = ' '
last_progress = 0
song_name = ' '
artist_names = ' '
read = ' '
song_uri = ' '
progress = 0
off = False
total_length = 100000000
current_song = sp.current_playback()
playing = current_song['is_playing']
print(playing)

while(True):
    try:
        current_song = sp.current_playback()
        off = False
        song_name = current_song['item']['name']
        current_position = current_song['progress_ms']
        album = current_song['item']['album']['name']
        artists = [artist for artist in current_song['item']['artists']]
        
        artist_names = ', '.join([artist['name'] for artist in artists])
        progress = int(current_position/total_length*100)
        if (last_song != song_name or last_artist != artist_names):
                print(song_name + " " + artist_names + " " + album)
                output = song_name + ':' + artist_names + ':' + album + ':'
                total_length = current_song['item']['duration_ms']
                test = "1"
                arduino_out.write(bytes(output, 'utf-8'))
        


    except:
        playing = False
        if (off == False):
            print("not connected")
            off = True
        time.sleep(5)


    #if (last_progress != progress):
        #print(progress)

    

#    if (arduino_in.in_waiting > 0):
#        print(arduino_in.readline())
    if (arduino_out.in_waiting > 0):
        read = arduino_out.readline()
        print(read)
        read = read.decode('utf-8')
        if ('p' in read):
            print("pause/play")
            if (playing == False):
                sp.start_playback()
                playing = True
            else:
                sp.pause_playback()
                playing = False
        elif ('f' in read):
            print("skip forward")
            sp.next_track()
        elif ('b' in read):
            print("skip back")
            sp.previous_track()
    
    last_progress = progress
    last_song = song_name
    last_artist = artist_names
    #time.sleep(0.25)

arduino_out.close()