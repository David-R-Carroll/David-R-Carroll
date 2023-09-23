import tkinter as tk
from tkinter import ttk
from tkmacosx import Button
from tkmacosx import Marquee
import tkinter.font as font
import socket
import time

################################################################################
#                                                                              #
#                  M5StickC Production Switcher Simulator.                     #
#                    Badly written by David R. Carroll.                        #
#                      Creative Commons C BY-NC 2023                           #
#                        This is the switcher panel.                           #
#                          It sends text commamnds to two M5StickC             #
#                            IoT development boards, that act as               #
#                              Profgram and preview monitors.                  #
#                                                                              #
################################################################################   


# Comment this out if you want to run the panel without M5StickC's running.
# Change the IPs to match the M5Stick C code.

# """
tn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tn.settimeout(5)
print()
print("Connecting to M5StickC PGM")
tn.connect(('192.168.2.240',21))

tn_PRV = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tn_PRV.settimeout(5)
print()
print("Connecting to M5StickC PRV")
tn_PRV.connect(('192.168.2.250',21))
# """


# root window
root = tk.Tk()
root.geometry('1000x500+400+100')
root.configure(background='#444')
root.resizable(False, False)
root.title('Switcher')


PushButtonFont = font.Font(family="Arial Black",size=26,)
MemoryButtonFont = font.Font(family="Arial",size=26,)
ButtonFont = font.Font(family="Arial", size=16)
Mnemonic_Font = font.Font(family="Arial", size=14)
ListBox_Font = font.Font(family="Arial", size=18)
panelMenuFont=font.Font(family="Courier New", size=18)
keyType_Font = font.Font(family="Arial", size=12)

panelButtonFont=font.Font(family="Zapf Dingbats", size=26)

PGM_FG="white"
PGM_GLOW = "blue"
SELECT_BUTTON_BG = "#59F"
BTN_FG="#111"
MNM_FG = "#8F8"
LBL_FG="#DDD"
LBL_BG ="#222"

#ttk.Style().configure("TreeStyle", foreground=LBL_BG, background="white")

PGM_Buttons = 9
MEM_Buttons = 9
PGM_Top = 200
PGM_Left = 20
PGM_x_Spacing = 60
PGM_y_Spacing = 50

MEM_Top = 50
MEM_Left = 20

KEY_button = [0] * PGM_Buttons 
MNM_label = [0] * PGM_Buttons 
PGM_button = [0] * PGM_Buttons
PST_button = [0] * PGM_Buttons
MEM_button = [0] * PGM_Buttons
storeRecallButton = [0] * 3
programEffectButton = [0] * 2
transTypeButton = [0] * 3
storeInclButton = [0] * 2
transInclButton = [0] * 2
panelMenuButton = [0] * 8
keyTypeButton = [0] * 3
memories = [[0],[0]]
for x in range(10):
    memories.insert(0,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])

Current_KEY_Button = 0
Current_PGM_Button = 0
Current_PST_Button = 0
Current_Recall_Button = 0
Current_Store_Button = 0
Current_TransIncl = 0
Current_TransType = 0
Current_TransKey = 0
Current_TransBkgd = 1
Current_KeyTally = 0
Current_PE_Button = 0
lastButtonPressed = -1
DVEXPOS = 0
DVEYPOS = 0
switcherVersion = "M5 Switcher v1.0\n "

Current_TransIncl = 1 # 1 BKGD 2 KEY 3 BKGD+KEY

TransRate = [['0', '30', '60', '120', '240'], ['0', '8', '4', '2', '1']]
Current_TransRate = 2

WipePattern = [[' ','Split-H    ', 'Race       ', 'Star       ', 'Checkers   '], ['0', '1', '2', '3', '4']]
Current_Wipe = 1

DVEPattern = [[' ', 'Push-R     ', '4 Box      ', 'Squeeze-H  ', 'Amgels     '], ['0', '1', '2', '3', '4']]
Current_DVE = 1

DVEKeySize = [[' ', '0     ', '20    ', '30    ', '40    ', '50    ', '60    ', '70    ', '80    ', '90    ', '100   '], [0, 0, 20,30, 40, 50, 60, 70, 80, 90, 100]]
Current_DVEKeySize = 3

DVEKeyPersp = [[' ', '-16','-8','0','+8','+16'], ['0', '1', '2', '3', '4', '5']]
Current_DVEKeyPersp = 3

#******************************************************************  Functions  ************************** 

def setPRV():
    global Current_PGM_Button
    global Current_PST_Button
    global Current_KEY_Button
    global Current_TransKey
    global Current_KeyTally
    global Current_TransBkgd
    global Current_TransIncl
    print("---------------------------- setPRV")

    sendPRV("KEY:" + str(Current_KEY_Button))

    if Current_TransIncl == 1: # BKGD only        
        sendPRV("KEYTALLY:" + str(Current_KeyTally))
        sendPRV("TRANSINCL:1")
        sendPRV("PST:" + str(Current_PST_Button))
        sendPRV("MECUT:0")
        SendDVESettings(2)

    if Current_TransIncl == 2: # Key only
        if Current_KeyTally == 1:
            sendPRV("KEYTALLY:0")
        else:
            sendPRV("KEYTALLY:1")

        sendPRV("TRANSINCL:1")                  
        SendDVESettings(2)
        sendPRV("PST:" + str(Current_PGM_Button))
        sendPRV("MECUT:0")

    if Current_TransIncl == 3: # BKGD + Key only
        if Current_KeyTally == 1:
            sendPRV("KEYTALLY:0")
        else:
            sendPRV("KEYTALLY:1")

        sendPRV("TRANSINCL:1")                  
        SendDVESettings(2)
        sendPRV("PST:" + str(Current_PST_Button))
        sendPRV("MECUT:0")

    pass

def send(command):
    tn.send(command.encode('ascii') + b"\r\n")
    print("PGM " + command)

    pass

def sendPRV(command):
    tn_PRV.send(command.encode('ascii') + b"\r\n")
    print("PRV " + command)

    pass

#******************************************************************  Define panel   ************************** 

keyTypeText = ('SELF', 'CK', 'DVE')
for i in range (3):
    keyTypeButton[i] = Button(root,font=keyType_Font,text = keyTypeText[i],borderless="true",fg=PGM_FG,bg=PGM_GLOW,height=40,width=40,command=lambda Current_KeyType=i: KeyTypePress(Current_KeyType))
    keyTypeButton[i].place(x = 27 + i * PGM_x_Spacing, y = 155)
    
for i in range (PGM_Buttons):
    KEY_button[i] = Button(root,font=PushButtonFont,text=i,borderless="true",fg=PGM_FG,bg=PGM_GLOW,height=50,width=50,command=lambda Current_KEY_Button=i: KEY_ButtonPress(Current_KEY_Button))
    KEY_button[i].place(x = PGM_Left + i * PGM_x_Spacing, y = PGM_Top)

    PGM_button[i] = Button(root,font=PushButtonFont,text=i,borderless="true", fg=PGM_FG,bg=PGM_GLOW,height=50,width=50,command=lambda Current_PGM_Button=i: PGM_ButtonPress(Current_PGM_Button))
    PGM_button[i].place(x = PGM_Left + i * PGM_x_Spacing, y = PGM_Top + 80)

    PST_button[i] = Button(root,font=PushButtonFont,text=i, borderless="true", fg=PGM_FG,bg=PGM_GLOW,height=50,width=50,command=lambda Current_PST_Button=i: PST_ButtonPress(Current_PST_Button))
    PST_button[i].place(x = PGM_Left + i * PGM_x_Spacing, y = 340)

for i in range (MEM_Buttons):
    MEM_button[i] = Button(root,font=MemoryButtonFont,text=i+1,borderless="true",fg=PGM_FG,bg=PGM_GLOW,height=50,width=50,command=lambda memoryButton=i: MEM_ButtonPress(memoryButton))
    if i == 8:
        MEM_button[i].configure(text = 'R State', font=ButtonFont, height=50, width=80)
    MEM_button[i].place(x = MEM_Left + i * PGM_x_Spacing, y = MEM_Top + PGM_y_Spacing * 0)

storeRecallButton[0] = Button(root,font=ButtonFont,text="RECALL",borderless="true",fg=PGM_FG,bg=PGM_GLOW,height=40,width=90,command=lambda Current_SR_Button=0: SR_ButtonPress(Current_SR_Button))
storeRecallButton[0].place(x = 585, y = 30)

storeRecallButton[1] = Button(root,font=ButtonFont,text="STORE",borderless="true",fg=PGM_FG,bg=PGM_GLOW,height=40,width=90,command=lambda Current_SR_Button=1: SR_ButtonPress(Current_SR_Button))
storeRecallButton[1].place(x = 585, y = 30 + 50)

storeRecallButton[2] = Button(root,font=ButtonFont,text="SAVE MEM",borderless="true",fg=PGM_FG,bg="red",height=0,width=0,command=lambda Current_SR_Button=2: SR_ButtonPress(Current_SR_Button))
storeRecallButton[2].place(x = 575, y = 30 + 100)

programEffectButton[0] = Button(root,font=ButtonFont,text="PGM",borderless="true",fg=PGM_FG,bg=PGM_GLOW,height=40,width=90,command=lambda Current_PE_Button=0: PE_ButtonPress(Current_PE_Button))
programEffectButton[0].place(x = 685+20, y = 30)

programEffectButton[1] = Button(root,font=ButtonFont,text="EFF DIS",borderless="true",fg=PGM_FG,bg=PGM_GLOW,height=40,width=90,command=lambda Current_PE_Button=1: PE_ButtonPress(Current_PE_Button))
programEffectButton[1].place(x = 785+10, y = 30)

CUT_button = Button(root, font=ButtonFont, text="CUT" ,borderless="true", fg=PGM_FG, bg=PGM_GLOW, height=50, width=70, command= lambda :CUT_ButtonPress())
CUT_button.place(x = 680, y = 340)

AUTO_button = Button(root, font=ButtonFont, text="AUTO" ,borderless="true", fg=PGM_FG, bg=PGM_GLOW, height=50, width=70, command=lambda :AUTO_ButtonPress())
AUTO_button.place(x = 760, y = 340)

transTypeButton[0] = Button(root, font=ButtonFont, text="DISS" ,borderless="true", fg=PGM_FG, bg=PGM_GLOW, height=50, width=70, command=lambda Current_TransType=0: TransTypePress(Current_TransType))
transTypeButton[0].place(x = 680, y = 280)

transTypeButton[1] = Button(root, font=ButtonFont, text="WIPE" ,borderless="true", fg=PGM_FG, bg=PGM_GLOW, height=50, width=70, command=lambda Current_TransType=1: TransTypePress(Current_TransType))
transTypeButton[1].place(x = 760, y = 280)

transTypeButton[2] = Button(root, font=ButtonFont, text="DVE" ,borderless="true", fg=PGM_FG, bg=PGM_GLOW, height=50, width=70, command=lambda Current_TransType=2: TransTypePress(Current_TransType))
transTypeButton[2].place(x = 840, y = 280)

transInclButton[0] = Button(root, font=ButtonFont, text="BKGD" ,borderless="true", fg=PGM_FG, bg=PGM_GLOW, height=50, width=70, command=lambda TransIncl=0: TransInclPress(TransIncl))
transInclButton[0].place(x = 680, y = 220)

transInclButton[1] = Button(root, font=ButtonFont, text="KEY" ,borderless="true", fg=PGM_FG, bg=PGM_GLOW, height=50, width=70, command=lambda TransIncl=1: TransInclPress(TransIncl))
transInclButton[1].place(x = 760, y = 220)

panelMenuText = Marquee(root, font=panelMenuFont,height=100,width=240,fg="#3F3", bg="#33F")
panelMenuText.place(x = 730-25, y = 50+40)
panelMenuText.stop(True)

# Up
panelMenuButton[0] = Button(root, font=panelButtonFont, text="\u23F6" ,borderless="true", fg=PGM_FG, bg=PGM_GLOW, height=30, width=30, command=lambda panelButton=1: panelButtonPress(panelButton))
panelMenuButton[0].place(x = 765-25, y = 110+30)

# Down
panelMenuButton[1] = Button(root, font=panelButtonFont, text="\u23F7" ,borderless="true", fg=PGM_FG, bg=PGM_GLOW, height=30, width=30, command=lambda panelButton=2: panelButtonPress(panelButton))
panelMenuButton[1].place(x = 765-25, y = 140+30)

# Left
panelMenuButton[6] = Button(root, font=panelButtonFont, text="\u23F4" ,borderless="true", fg=PGM_FG, bg=PGM_GLOW, height=30, width=30, command=lambda panelButton=7: panelButtonPress(panelButton))
panelMenuButton[6].place(x = 735-25, y = 125+30)

# Right
panelMenuButton[7] = Button(root, font=panelButtonFont, text="\u23F5" ,borderless="true", fg=PGM_FG, bg=PGM_GLOW, height=30, width=30, command=lambda panelButton=8: panelButtonPress(panelButton))
panelMenuButton[7].place(x = 795-25, y = 125+30)

# Second Up/Down
panelMenuButton[2] = Button(root, font=panelButtonFont, text="\u23F6" ,borderless="true", fg=PGM_FG, bg=PGM_GLOW, height=30, width=30, command=lambda panelButton=3: panelButtonPress(panelButton))
panelMenuButton[2].place(x = 850-25, y = 110+30)

panelMenuButton[3] = Button(root, font=panelButtonFont, text="\u23F7" ,borderless="true", fg=PGM_FG, bg=PGM_GLOW, height=30, width=30, command=lambda panelButton=4: panelButtonPress(panelButton))
panelMenuButton[3].place(x = 850-25, y = 140+30)

# Third Up/Down
panelMenuButton[4] = Button(root, font=panelButtonFont, text="\u23F6" ,borderless="true", fg=PGM_FG, bg=PGM_GLOW, height=30, width=30, command=lambda panelButton=5: panelButtonPress(panelButton))
panelMenuButton[4].place(x = 910-25, y = 110+30)

panelMenuButton[5] = Button(root, font=panelButtonFont, text="\u23F7" ,borderless="true", fg=PGM_FG, bg=PGM_GLOW, height=30, width=30, command=lambda panelButton=6: panelButtonPress(panelButton))
panelMenuButton[5].place(x = 910-25, y = 140+30)


KEY_Label = tk.Label(root,text="KEY",font=ButtonFont, fg=LBL_FG, bg=LBL_BG) 
KEY_Label.place(x = PGM_Left + PGM_x_Spacing * PGM_Buttons-5, y = PGM_Top + 13)

Mnemonics = ('Wash', 'Bars', 'Anna', 'Flag', 'Ancor', 'Stana', 'Angels', 'Vase', 'Grid')
for i in range (PGM_Buttons):
    MNM_label[i] = tk.Label(root,font=Mnemonic_Font,bd= 2,text=Mnemonics[i],fg=MNM_FG,bg=LBL_BG,height=1,width=6)
    MNM_label[i].place(x = PGM_Left + i * PGM_x_Spacing, y = PGM_Top + 55)

PGM_Label = tk.Label(root,text="PGM",font=ButtonFont, fg=LBL_FG, bg=LBL_BG) 
PGM_Label.place(x = PGM_Left + PGM_x_Spacing * PGM_Buttons-5, y = PGM_Top + 13 + 80)

PST_Label = tk.Label(root,text="PST",font=ButtonFont, fg=LBL_FG, bg=LBL_BG) 
PST_Label.place(x = PGM_Left + PGM_x_Spacing * PGM_Buttons-5, y = 353)

MEM_Label = tk.Label(root,text="MEMORIES",font=ButtonFont, fg=LBL_FG, bg=LBL_BG) 
MEM_Label.place(x = 300, y = 15)

def Close_App():
    print( "Closing switcher")
    tn.close
    tn_PRV.close
    root.destroy()
    quit()

root.protocol("WM_DELETE_WINDOW", Close_App)


def KeyTypePress(Current_KeyType):
    global DVEKeySize
    global Current_DVEKeySize
    global DVEKeyPersp
    global Current_DVEKeyPersp
    global lastButtonPressed
    global switcherVersion
    global DVEXPOS
    global DVEYPOS

    send("KEYTYPE:" + str(Current_KeyType))
    lastButtonPressed = Current_KeyType + 3
    print("lastButtonPressed: ", lastButtonPressed)

    for i in range (3):
        if i == Current_KeyType:
            keyTypeButton[i].config(bg=SELECT_BUTTON_BG)
        else:
            keyTypeButton[i].config(bg=PGM_GLOW)

    if Current_KeyType == 0:
        panelMenuText.config(text=switcherVersion)

    if Current_KeyType == 1:
        panelMenuText.config(text=switcherVersion)

    if Current_KeyType == 2:
        panelButtonPress(0)
    pass


def KEY_ButtonPress(Button):
    global Current_KEY_Button
    global KEY_button

    Current_KEY_Button = Button
    send("KEY:" + str(Current_KEY_Button))
    sendPRV("KEY:" + str(Current_KEY_Button))
    setButton(KEY_button,Current_KEY_Button,Current_KeyTally)
    pass

def setButton(buttonRow,button,tally):
    for i in range (PGM_Buttons):
        if i==button:
            if tally == 1:
                buttonRow[i].config(bg='red')
                print("red " + str(button))
            else:
                if buttonRow == PST_button:
                    buttonRow[i].config(bg="green")
                    print("green " + str(button))
                else:
                    buttonRow[i].config(bg=SELECT_BUTTON_BG)
        else:
            buttonRow[i].config(bg=PGM_GLOW)
    pass


def PGM_ButtonPress(button):
    global PGM_button
    global Current_PGM_Button
    Current_PGM_Button = button

    send("BKGD:" + str(Current_PGM_Button))
    setPRV()
    setButton(PGM_button,button,1)

    pass


def PST_ButtonPress(button):
    global Current_PST_Button
    global PST_button
    Current_PST_Button = button

    send("PST:" + str(Current_PST_Button))
    setPRV()

    setButton(PST_button,button,0)
    
    pass


def MEM_ButtonPress(memoryButton):
    global memories
    global Current_Recall_Button
    global Current_Store_Button
    global Current_SR_Button
    global Current_PE_Button
    global Current_KEY_Button
    global Current_PGM_Button
    global Current_PST_Button
    global Current_TransKey
    global Current_TransBkgd
    global Current_TransType
    global Current_TransIncl
    global Current_KeyTally
    global Current_TransRate
    global Current_Wipe
    global Current_DVE
    global DVEXPOS
    global DVEYPOS
    global Current_DVEKeySize
    global Current_DVEKeyPersp
    global Current_PE_Button
    global lastButtonPressed
    
    if memoryButton == 8 :    #RState
       storeRecallButton[2].config(height=0,width=0)
       rState() 
    else:
        if Current_SR_Button == 0:                          # Recall mode
            Current_Recall_Button = memoryButton
            print("               Recall MEM:",Current_Recall_Button)
            for i in range (MEM_Buttons-1):
                if i==Current_Recall_Button:
                    MEM_button[i].config(font=PushButtonFont)
                else:
                    MEM_button[i].config(font=MemoryButtonFont)
                
                if memories[i][0] == 1:
                    MEM_button[i].config(bg="purple")
                else:
                    MEM_button[i].config(bg=PGM_GLOW)
                                                            # Recall the memmory
            
            Current_KEY_Button = memories[Current_Recall_Button][1]
            Current_KeyTally  = memories[Current_Recall_Button][8]
            send("Mem_KEY:" + str(Current_KEY_Button))
            setButton(KEY_button,Current_KEY_Button,Current_KeyTally)
            
            Current_PGM_Button = memories[Current_Recall_Button][2]
            send("Mem_BKGD:" + str(Current_PGM_Button))
            setButton(PGM_button,Current_PGM_Button,1)

            Current_PST_Button = memories[Current_Recall_Button][3] 
            send("Mem_PST:" + str(Current_PST_Button))
            setButton(PST_button,Current_PST_Button,0)

            Current_TransKey = memories[Current_Recall_Button][4]
            Current_TransBkgd = memories[Current_Recall_Button][5]
            Current_TransType = memories[Current_Recall_Button][6]
            send("Mem_TRANSTYPE:" + str(Current_TransType))
            for i in range (3):
                if i == Current_TransType:
                    transTypeButton[i].config(bg=SELECT_BUTTON_BG)
                else:
                    transTypeButton[i].config(bg=PGM_GLOW)

            Current_TransIncl = memories[Current_Recall_Button][7]

            if Current_TransIncl == 1: 
                transInclButton[0].config(bg=SELECT_BUTTON_BG)
                transInclButton[1].config(bg=PGM_GLOW)

            if Current_TransIncl == 2: 
                transInclButton[0].config(bg=PGM_GLOW)
                transInclButton[1].config(bg=SELECT_BUTTON_BG)

            if Current_TransIncl == 3: 
                transInclButton[0].config(bg=SELECT_BUTTON_BG)
                transInclButton[1].config(bg=SELECT_BUTTON_BG)
            
            send("Mem_TRANSINCL:" + str(Current_TransIncl))
            send("Mem_KEYTALLY:" + str(Current_KeyTally))

            Current_TransRate = memories[Current_Recall_Button][9]
            send("Mem_TRANSDIR:" + str(Current_TransRate))
            if lastButtonPressed == 0: #Dissolve
                panelMenuText.config(text='Trans Rate:\n' + TransRate[0][Current_TransRate])

            Current_Wipe = memories[Current_Recall_Button][10]
            send("Mem_TRANSWIPE:" + str(Current_Wipe))
            Current_DVE = memories[Current_Recall_Button][11]
            send("Mem_TRANSDVE:" + str(Current_DVE))

            DVEXPOS  = memories[Current_Recall_Button][12]
            DVEYPOS  = memories[Current_Recall_Button][13]
            Current_DVEKeySize = memories[Current_Recall_Button][14]
            Current_DVEKeyPersp = memories[Current_Recall_Button][15]
            
            #Current_PE_Button = memories[Current_Recall_Button][16]
            #PE_ButtonPress(Current_PE_Button)

            SendDVESettings(1)

            panelButtonPress(0) #Refresh display

            if Current_PE_Button == 0:                              # PGM recall
                send("PGM_RECALL:" + str(memoryButton))
            else:
                send("EFF_RECALL:" + str(memoryButton))

            temp = 'R:' + str(memoryButton) + '- '
            for j in range(20):
                temp = temp + str(j) + ":" + str(memories[memoryButton][j]) + ','
            print(temp)
            setPRV()

        if Current_SR_Button == 1 or Current_SR_Button == 2:         # Store mode
            storeRecallButton[2].config(height=40,width=110)
            Current_Store_Button = memoryButton
            for i in range (MEM_Buttons-1):
                if i==Current_Store_Button:
                    MEM_button[i].config(bg=SELECT_BUTTON_BG)                
                else:
                    if memories[i][0] == 1:
                        MEM_button[i].config(bg="purple")
                    else:
                        MEM_button[i].config(bg=PGM_GLOW)
    
    pass


def SR_ButtonPress(SR_Button):
    global Current_Recall_Button
    global Current_Store_Button
    global Current_SR_Button
    global MEM_Buttons

    Current_SR_Button = SR_Button
    if Current_SR_Button == 0:                          # Recall
        storeRecallButton[2].config(height=0,width=0)
        storeRecallButton[0].config(bg=SELECT_BUTTON_BG)
        storeRecallButton[1].config(bg=PGM_GLOW)

        for i in range (MEM_Buttons-1):
            if memories[i][0] == 1:
                MEM_button[i].config(bg="purple")
            else:
                MEM_button[i].config(bg=PGM_GLOW)
    
    if Current_SR_Button == 1:                          #Store
        storeRecallButton[0].config(bg=PGM_GLOW)
        storeRecallButton[1].config(bg=SELECT_BUTTON_BG)

        for i in range (MEM_Buttons-1):
            if memories[i][0] == 1:
                MEM_button[i].config(bg="purple")
            else:
                MEM_button[i].config(bg=PGM_GLOW)
    
    if Current_SR_Button == 2:                          #Save Mem
        storeRecallButton[2].config(height=0,width=0)
        print("Store MEM:",Current_Store_Button)
        memories[Current_Store_Button][0] = 1 # Is stored = true
        memories[Current_Store_Button][1] = Current_KEY_Button
        memories[Current_Store_Button][2] = Current_PGM_Button 
        memories[Current_Store_Button][3] = Current_PST_Button 
        memories[Current_Store_Button][4] = Current_TransKey
        memories[Current_Store_Button][5] = Current_TransBkgd
        memories[Current_Store_Button][6] = Current_TransType
        memories[Current_Store_Button][7] = Current_TransIncl
        memories[Current_Store_Button][8] = Current_KeyTally
        memories[Current_Store_Button][9] = Current_TransRate
        memories[Current_Store_Button][10] = Current_Wipe
        memories[Current_Store_Button][11] = Current_DVE
        memories[Current_Store_Button][12] = DVEXPOS 
        memories[Current_Store_Button][13] = DVEYPOS 
        memories[Current_Store_Button][14] = Current_DVEKeySize 
        memories[Current_Store_Button][15] = Current_DVEKeyPersp 
        memories[Current_Store_Button][16] = Current_PE_Button 
        
        temp = 'S:' + str(Current_Store_Button) + '- '
        for j in range(20):
            temp = temp + str(j) + ":" + str(memories[Current_Store_Button][j]) + ','
        print(temp)
    
        #np.savetxt('Switcher_Settings.csv', memories, delimiter=',')
        F = open('Switcher_Settings.csv','wt')
        for i in range(10):
            temp = ""
            for j in range(20):
                temp = temp + str(memories[i][j]) + ','
            temp = temp + '\r'
            F.write(temp)
        F.close()    

    pass


def PE_ButtonPress(PE_Button):
    global Current_PE_Button
    global programEffectButton

    Current_PE_Button = PE_Button

    if Current_PE_Button == 0: # Program Recall
        programEffectButton[0].config(bg=SELECT_BUTTON_BG)
        programEffectButton[1].config(bg=PGM_GLOW)
    else:
        programEffectButton[0].config(bg=PGM_GLOW)
        programEffectButton[1].config(bg=SELECT_BUTTON_BG)


def AUTO_ButtonPress():
    print("--------------------------MEAUTO")
    global Current_KEY_Button
    global KEY_button
    global Current_PGM_Button
    global Current_PST_Button
    global Current_TransIncl
    global Current_KeyTally
    global Current_TransType
    global Current_DVE
    global Current_Wipe
    global panelRate

    send("TRANSINCL:" + str(Current_TransIncl))

    if Current_TransIncl > 1: # key inclulde on (2 or 3)
        Current_KeyTally = int(not Current_KeyTally)

    if Current_KeyTally == 1:
        send("KEYTALLY:1")
        SendDVESettings(0)
        KEY_button[Current_KEY_Button].config(bg='red')
    else:
        send("KEYTALLY:0")
        KEY_button[Current_KEY_Button].config(bg=SELECT_BUTTON_BG)

    send("TRANSTYPE:" + str(Current_TransType))
    send("TRANSDVE:" + str(Current_DVE))
    send("TRANSWIPE:" + str(Current_Wipe))
    send("TRANSDIR:" + "1")

    SendDVESettings(0)

    setButton(PST_button,Current_PST_Button,1)

    send("MEAUTO:" + str(TransRate[1][Current_TransRate]))
    
    time.sleep(0.016*int(TransRate[0][Current_TransRate]))

    if Current_TransIncl != 2:
        temp = Current_PGM_Button
        Current_PGM_Button = Current_PST_Button
        Current_PST_Button = temp 
        setButton(PGM_button,Current_PGM_Button,1)
        setButton(PST_button,Current_PST_Button,0)
    setPRV()

    pass


def CUT_ButtonPress():
    print("--------------------------MECUT")
    global Current_KEY_Button
    global Current_PGM_Button
    global Current_PST_Button
    global Current_TransIncl
    global Current_KeyTally
    global KEY_button
    
    print("Current_TransIncl: " + str(Current_TransIncl))
    print("Current_KeyTally in: " + str(Current_KeyTally))
    print("Current_KEY_Button: " + str(Current_KEY_Button))

    send("TRANSINCL:" + str(Current_TransIncl))
    if Current_TransIncl > 1: # key inclulde on (2 or 3)
        Current_KeyTally = int(not Current_KeyTally)

    if Current_KeyTally == 1:
        send("KEYTALLY:1")
        SendDVESettings(0)
        KEY_button[Current_KEY_Button].config(bg='red')
    else:
        send("KEYTALLY:0")
        KEY_button[Current_KEY_Button].config(bg=SELECT_BUTTON_BG)

    send("MECUT:0")

    if Current_TransIncl != 2:
        temp = Current_PGM_Button
        Current_PGM_Button = Current_PST_Button
        Current_PST_Button = temp
        setButton(PGM_button,Current_PGM_Button,1)
        setButton(PST_button,Current_PST_Button,0)
        
    setPRV()

    pass


def TransTypePress(TransType):
    global Current_TransType
    global TransRate
    global Current_TransRate
    global panelMenuText
    global lastButtonPressed

    Current_TransType = TransType
    lastButtonPressed = TransType
    panelButtonPress(0) # set the panel menu
    for i in range (3):
        if i == TransType:
            transTypeButton[i].config(bg=SELECT_BUTTON_BG)
        else:
            transTypeButton[i].config(bg=PGM_GLOW)
        
    pass


def TransInclPress(TransIncl):
    global Current_TransIncl
    global Current_TransBkgd
    global Current_TransKey

    if TransIncl == 0:
        Current_TransBkgd = int(not Current_TransBkgd) 
        if Current_TransBkgd == 1:
            transInclButton[0].config(bg=SELECT_BUTTON_BG)
        else:
            transInclButton[0].config(bg=PGM_GLOW)

    if TransIncl == 1:
        Current_TransKey = int(not Current_TransKey) 
        if Current_TransKey == 1:
            transInclButton[1].config(bg=SELECT_BUTTON_BG)
        else:
            transInclButton[1].config(bg=PGM_GLOW)

    if Current_TransBkgd == 0 and Current_TransKey == 0:
        Current_TransBkgd = 1
        transInclButton[0].config(bg=SELECT_BUTTON_BG)
    Current_TransIncl = Current_TransBkgd + Current_TransKey * 2

    print("Current_TransIncl= " + str(Current_TransIncl))
    setPRV()
    pass


def panelButtonPress(panelMenuButton):
    global Current_TransType
    global TransRate
    global Current_TransRate
    global Current_Wipe
    global WipePattern
    global Current_DVE
    global DVEPattern
    global panelMenuText
    global lastButtonPressed
    #global Current_DVEKeyPos
    global Current_DVEKeySize
    global Current_DVEKeyPersp
    #global DVEKeyPos
    global DVEKeySize
    global DVEKeyPersp
    global DVEXPOS
    global DVEYPOS
    
    if lastButtonPressed == 0: #Dissolve
        if panelMenuButton == 3:
            Current_TransRate = Current_TransRate + 1
            if Current_TransRate > 4:
                Current_TransRate = 4
        if panelMenuButton == 4:
            Current_TransRate = Current_TransRate - 1
            if Current_TransRate < 1:
                Current_TransRate = 1

        print("Panel-Current_TransRate:", Current_TransRate)    
        panelMenuText.config(text='           Rate:\n' + "           " + TransRate[0][Current_TransRate])

    if lastButtonPressed == 1: #Wipe
        if panelMenuButton == 1:
            Current_Wipe = Current_Wipe + 1
            if Current_Wipe > 4:
                Current_Wipe = 4
        if panelMenuButton == 2:
            Current_Wipe = Current_Wipe - 1
            if Current_Wipe < 1:
                Current_Wipe = 1
        if panelMenuButton == 3:
            Current_TransRate = Current_TransRate + 1
            if Current_TransRate > 4:
                Current_TransRate = 4
        if panelMenuButton == 4:
            Current_TransRate = Current_TransRate - 1
            if Current_TransRate < 1:
                Current_TransRate = 1

        panelMenuText.config(text='Wipe:      Rate:\n' + WipePattern[0][Current_Wipe] + TransRate[0][Current_TransRate])

    if lastButtonPressed == 2: #DVE
        if panelMenuButton == 1:
            Current_DVE = Current_DVE + 1
            if Current_DVE > 4:
                Current_DVE = 4
        if panelMenuButton == 2:
            Current_DVE = Current_DVE - 1
            if Current_DVE < 1:
                Current_DVE = 1
        if panelMenuButton == 3:
            Current_TransRate = Current_TransRate + 1
            if Current_TransRate > 4:
                Current_TransRate = 4
        if panelMenuButton == 4:
            Current_TransRate = Current_TransRate - 1
            if Current_TransRate < 1:
                Current_TransRate = 1

        panelMenuText.config(text='DVE:       Rate:\n' + DVEPattern[0][Current_DVE] + TransRate[0][Current_TransRate])

    if lastButtonPressed == 5: #     Key DVE
        if panelMenuButton == 1: #   Pos up
            DVEYPOS = DVEYPOS - 10
            if DVEYPOS < -80:
                DVEYPOS = -80
            
        if panelMenuButton == 2: #   Pos down
            DVEYPOS = DVEYPOS + 10
            if DVEYPOS > 80:
                DVEYPOS = 80

        if panelMenuButton == 7: #   Pos left
            DVEXPOS = DVEXPOS - 10
            if DVEXPOS < -160:
                DVEXPOS = -160
            
        if panelMenuButton == 8: #   Pos right
            DVEXPOS = DVEXPOS + 10
            if DVEXPOS > 160:
                DVEXPOS = 160

        if panelMenuButton == 3: #                 Size
            Current_DVEKeySize = Current_DVEKeySize + 1
            if Current_DVEKeySize > 10:
                Current_DVEKeySize = 10

        if panelMenuButton == 4:
            Current_DVEKeySize = Current_DVEKeySize - 1
            if Current_DVEKeySize < 1:
                Current_DVEKeySize = 1

        if panelMenuButton == 5: #                  Persp
            Current_DVEKeyPersp = Current_DVEKeyPersp + 1
            if Current_DVEKeyPersp > 5:
                Current_DVEKeyPersp = 5

        if panelMenuButton == 6:
            Current_DVEKeyPersp = Current_DVEKeyPersp - 1
            if Current_DVEKeyPersp < 1:
                Current_DVEKeyPersp = 1
        
        if DVEXPOS !=0:
            xy = (' {:+} '.format(DVEXPOS))
        else:
            xy = "  0  "

        if DVEYPOS !=0:
            xy = xy + ('{:+}   '.format(DVEYPOS))
        else:
            xy = xy + " 0    "

        temp = '  x:  y:  Size: Persp:\n' \
            + xy \
            + DVEKeySize[0][Current_DVEKeySize] \
            + DVEKeyPersp[0][Current_DVEKeyPersp]
        panelMenuText.config(text=temp)

        # if Current_KeyTally == 1:
        SendDVESettings(0)
        send("TRANSINCL:2")
        send("MECUT:0")

        SendDVESettings(2)
        sendPRV("TRANSINCL:2")
        sendPRV("MECUT:0")

    pass


def SendDVESettings(recallType):
    print("--------------------------SendDVESettings")
    global DVEKeySize
    global Current_DVEKeySize
    #global DVEKeyPos
    #global Current_DVEKeyPos
    global DVEKeyPersp
    global Current_DVEKeyPersp
    global DVEXPOS
    global DVEYPOS
    
    DVESIZE = DVEKeySize[1][Current_DVEKeySize]
   
    if recallType == 0: 
        send("DVESIZE:" + str(DVESIZE)) # Manual Recall
        send("DVEXPOS:" + str(int(DVEXPOS)))
        send("DVEYPOS:" + str(int(DVEYPOS)))
        send("DVEPERSP:" + str(DVEKeyPersp[0][Current_DVEKeyPersp]))

    if recallType == 1: 
        send("Mem_DVESIZE:" + str(DVESIZE)) # Memory Recall
        send("Mem_DVEXPOS:" + str(int(DVEXPOS)))
        send("Mem_DVEYPOS:" + str(int(DVEYPOS)))
        send("Mem_DVEPERSP:" + str(DVEKeyPersp[0][Current_DVEKeyPersp]))

    if recallType == 2: 
        sendPRV("DVESIZE:" + str(DVESIZE)) # Manual Recall
        sendPRV("DVEXPOS:" + str(int(DVEXPOS)))
        sendPRV("DVEYPOS:" + str(int(DVEYPOS)))
        sendPRV("DVEPERSP:" + str(DVEKeyPersp[0][Current_DVEKeyPersp]))

    pass


def rState():
    print("--------------------------R State")
    global Current_TransType
    global Current_TransRate
    global Current_TransIncl
    global Current_KEY_Button
    global Current_PST_Button
    global Current_PGM_Button
    global switcherVersion
    global lastButtonPressed
    global Current_TransKey
    global Current_TransBkgd
    global Current_PE_Button
    global Current_KeyTally
    global Current_SR_Button
    global Current_Wipe
    global Current_DVE
    global Current_DVEKeySize
    global Current_DVEKeyPersp
    global DVEXPOS
    global DVEYPOS
    global MEM_Buttons
    global MEM_button
    global keyTypeButton
    global storeRecallButton
    global programEffectButton
    global panelMenuText
    global switcherVersion
    
    F = open('Switcher_Settings.csv','rt')
    temp = F.read()
    valueStart = 0
    
    for i in range(10):                    # Recall memories 
        for j in range(20):
            if valueStart != 0:
                valueStart = temp.find(',',valueStart)
                if valueStart >= 0:
                    valueEnd = temp.find(',',valueStart+1)
                    memories[i][j] = int(temp[valueStart+1:valueEnd])
                    valueStart = valueEnd
                else:
                    print('End of file')
                    break
            else:
                memories[i][j] = int(temp[0:1])
                valueStart = 1
    F.close()
    
    keyTypeButton[2].config(bg=SELECT_BUTTON_BG)
    Current_KeyTally = 0
    send("KEYTALLY:0")
    sendPRV("KEYTALLY:0")

    Current_KEY_Button = 0
    send("KEY:0")
    sendPRV("KEY:0")
    setButton(KEY_button,0,0)

    Current_PGM_Button = 0
    send("BKGD:0")
    sendPRV("BKGD:0")
    setButton(PGM_button,0,0)

    Current_PST_Button = 0
    send("PST:0")
    sendPRV("PST:0")
    setButton(PST_button,0,0)

    Current_SR_Button = 0
    storeRecallButton[0].config(bg=SELECT_BUTTON_BG)
    Current_PE_Button = 0
    programEffectButton[0].config(bg=SELECT_BUTTON_BG)
    programEffectButton[1].config(bg=PGM_GLOW)

    transInclButton[0].config(bg=SELECT_BUTTON_BG)
    transInclButton[1].config(bg=PGM_GLOW)

    Current_TransKey = 0
    Current_TransBkgd = 1
    Current_TransIncl = 1
    send("TRANSINCL:1")
    sendPRV("TRANSINCL:1")

    Current_TransRate = 2
    send("Mem_TRANSDIR:" + str(Current_TransRate))

    Current_Wipe = 1
    Current_DVE = 1
    DVEXPOS = 0
    DVEYPOS = 0
    Current_DVEKeySize = 3
    Current_DVEKeyPersp = 3
    
    transTypeButton[0].config(bg=SELECT_BUTTON_BG)
    transTypeButton[1].config(bg=PGM_GLOW)
    transTypeButton[2].config(bg=PGM_GLOW)
    Current_TransType = 0
    lastButtonPressed = -1 # No key

    panelMenuText.config(text=switcherVersion)
            
    panelMenuText.config(text=switcherVersion)

    for i in range (MEM_Buttons-1):
        if memories[i][0] == 1:
            MEM_button[i].config(bg="purple")
        else:
            MEM_button[i].config(bg=PGM_GLOW)
    
    pass
    
rState()

root.mainloop()
