from audioop import ulaw2lin
import numpy as np
from os import urandom
from telnetlib import TUID
from xml.dom.expatbuilder import theDOMImplementation
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import sys
import copy
from collections import defaultdict
from mpl_toolkits.mplot3d.axes3d import Axes3D

#ex:python3 draw.py case1.txt output1.txt output




png_size = (16,12)


Draw_Initial_Chip = False
Draw_Partition_Chip = False

Draw_3D_Chip = False


system_arg = sys.argv
library_file = system_arg[1]
position_file = system_arg[2]
initial_cell = []
Terminal = []
final_cell = []

print("system_arg")
if("-i" in system_arg) or ("-I" in system_arg):
    Draw_Initial_Chip = True
if("-p" in system_arg) or ("-P" in system_arg):
    Draw_Partition_Chip = True

#------------------------parser for input
txt_name = library_file
fread = open(txt_name,'r')
f = fread.read().split("\n")


Die_size = [] #x1 y1 x2 y2
Cell_number = 0

i = 0
while(i<len(f)):

    ss = f[i].split(" ")


    #------------------------------------------------------------------ Die_size
    if(ss[0]=="DieSize"):
        Die_size = [0,0,int(ss[1]),int(ss[2])]  
    #------------------------------------------------------------------ DieRows
    elif(ss[0]=="DieRows"):
        DieRow_height = int(ss[1])
        DieRows = int(ss[2])
    #------------------------------------------------------------------ for NumCell
    elif(ss[0]=="Terminal"): #Terminal 0
        Terminal_number = int(ss[1])
        for j in range( Terminal_number ):
            i+=1
            # T0 0 0 2 2
            ss1 = f[i].split(" ")
            Terminal.append([int(ss1[1]),int(ss1[2]),int(ss1[3]),int(ss1[4])])
            
    #------------------------------------------------------------------ for NumCell
    elif(ss[0]=="NumCell"): #NumCell 4
        Cell_number = int(ss[1])
        for j in range( Cell_number ):
            i+=1
            # C0 0 3 10 10
            ss1 = f[i].split(" ")
            initial_cell.append([int(ss1[1]),int(ss1[2]),int(ss1[3]),int(ss1[4])])
            
    i+=1



#------------------------parser for output
txt_name = position_file
fread = open(txt_name,'r')
f = fread.read().split("\n")


i = 0
top_cell = {}
bottom_cell = {}

while(i < Cell_number):
    # C0 0 10 0
    ss = f[i].split(" ")
    cell_index = int(ss[0][1:])
    cell_x_position = int(ss[1])
    cell_y_position = int(ss[2])
    chip_layer = int(ss[3])

    if(chip_layer == 1): 
        top_cell[cell_index] = [cell_index,cell_x_position,cell_y_position]
    else:
        bottom_cell[cell_index] = [cell_index,cell_x_position,cell_y_position]


    i+=1


Die_width = Die_size[2]- Die_size[0]
Die_height = Die_size[3]- Die_size[1]



#----------------------------------------------plot bottom picture
fig = plt.figure(figsize=png_size)
plt.title("Bottom Chip",fontsize="xx-large",fontweight="heavy")
bottom_chip_ax = fig.add_subplot(111)
bottom_chip_ax.set_xbound(Die_size[0],Die_size[2])
bottom_chip_ax.set_ybound(Die_size[1],Die_size[3])

#for Terminal
for i in range(len(Terminal)) :
    cell_index = i
    bottom_chip_ax.add_patch(
            patches.Rectangle(  #{'TA': [(7, 10), (14, 10)], 'TB': [(7, 15),  (16, 15)]}
                (Terminal[cell_index][0],Terminal[cell_index][1]),
                Terminal[cell_index][2],
                Terminal[cell_index][3],
                fill = True,
                edgecolor = '#990',
                facecolor = '#990',
                alpha = 0.7 #0.3 original
            )
        )

#for cell
for i in  list(bottom_cell.keys()) :
    cell_index = i
    bottom_chip_ax.add_patch(
            patches.Rectangle( #{'TA': [(7, 10), (14, 10)], 'TB': [(7, 15),  (16, 15)]}
                (bottom_cell[i][1], bottom_cell[i][2]),
                initial_cell[cell_index][2],
                initial_cell[cell_index][3],
                fill = True,
                edgecolor = '#333',
                facecolor = '#777',
                alpha = 0.5 #0.3 original
            )
        )

#for row line
for i in range(DieRows):
    bottom_chip_ax.add_patch(
    patches.Rectangle( 
            (Die_size[0], Die_size[1]+(DieRow_height*i) ),
            Die_width, 
            1,
            fill = True,
            edgecolor = '#900',
            facecolor = '#900',
            alpha = 0.5 #0.3 original
        )
    )

#ax.autoscale_view()
plt.savefig(system_arg[3]+"_bottom.png")
#plt.show()

#----------------------------------------------plot top picture
fig = plt.figure(figsize=png_size)
plt.title("Top Chip",fontsize="xx-large",fontweight="heavy")
top_chip_ax = fig.add_subplot(111)
top_chip_ax.set_xbound(Die_size[0],Die_size[2])
top_chip_ax.set_ybound(Die_size[1],Die_size[3])

#for Terminal
for i in range(len(Terminal)) :
    cell_index = i
    top_chip_ax.add_patch(
            patches.Rectangle(  #{'TA': [(7, 10), (14, 10)], 'TB': [(7, 15),  (16, 15)]}
                (Terminal[cell_index][0],Terminal[cell_index][1]),
                Terminal[cell_index][2],
                Terminal[cell_index][3],
                fill = True,
                edgecolor = '#990',
                facecolor = '#990',
                alpha = 0.7 #0.3 original
            )
        )
#for cell
for i in list(top_cell.keys()) :
    cell_index = i
    top_chip_ax.add_patch(
            patches.Rectangle(  #{'TA': [(7, 10), (14, 10)], 'TB': [(7, 15),  (16, 15)]}
                (top_cell[i][1], top_cell[i][2]),
                initial_cell[cell_index][2],
                initial_cell[cell_index][3],
                fill = True,
                edgecolor = '#333',
                facecolor = '#777',
                alpha = 0.3 #0.3 original
            )
        )

#for row line
for i in range(DieRows):
    top_chip_ax.add_patch(
    patches.Rectangle( 
            (Die_size[0], Die_size[1]+(DieRow_height*i) ),
            Die_width, 
            1,
            fill = True,
            edgecolor = '#900',
            facecolor = '#900',
            alpha = 0.5 #0.3 original
        )
    )


#ax.autoscale_view()
plt.savefig(system_arg[3]+"_top.png")
#plt.show()






#----------------------------------------------plot initial picture
if(Draw_Initial_Chip):

    fig = plt.figure(figsize=png_size)
    plt.title("Initial Chip",fontsize="xx-large",fontweight="heavy")
    top_chip_ax = fig.add_subplot(111)
    top_chip_ax.set_xbound(Die_size[0],Die_size[2])
    top_chip_ax.set_ybound(Die_size[1],Die_size[3])

    #for Terminal
    for i in range(len(Terminal)) :
        cell_index = i
        top_chip_ax.add_patch(
                patches.Rectangle(  #{'TA': [(7, 10), (14, 10)], 'TB': [(7, 15),  (16, 15)]}
                    (Terminal[cell_index][0],Terminal[cell_index][1]),
                    Terminal[cell_index][2],
                    Terminal[cell_index][3],
                    fill = True,
                    edgecolor = '#990',
                    facecolor = '#990',
                    alpha = 0.7 #0.3 original
                )
            )

    #for cell
    for i in range(len(initial_cell)) :
        cell_index = i
        top_chip_ax.add_patch(
                patches.Rectangle(  #{'TA': [(7, 10), (14, 10)], 'TB': [(7, 15),  (16, 15)]}
                    (initial_cell[cell_index][0],initial_cell[cell_index][1]),
                    initial_cell[cell_index][2],
                    initial_cell[cell_index][3],
                    fill = True,
                    edgecolor = '#333',
                    facecolor = '#777',
                    alpha = 0.3 #0.3 original
                )
            )

    #for row line
    for i in range(DieRows):
        top_chip_ax.add_patch(
        patches.Rectangle( 
                (Die_size[0], Die_size[1]+(DieRow_height*i) ),
                Die_width, 
                1,
                fill = True,
                edgecolor = '#900',
                facecolor = '#900',
                alpha = 0.5 #0.3 original
            )
        )

    #ax.autoscale_view()
    plt.savefig(system_arg[3]+"_initial.png")
    #plt.show()


#----------------------------------------------plot partition picture
if(Draw_Partition_Chip):

    #----------------------------------------------plot bottom picture
    fig = plt.figure(figsize=png_size)
    plt.title("Partition Bottom Chip",fontsize="xx-large",fontweight="heavy")
    bottom_chip_ax = fig.add_subplot(111)
    bottom_chip_ax.set_xbound(Die_size[0],Die_size[2])
    bottom_chip_ax.set_ybound(Die_size[1],Die_size[3])

    #for Terminal
    for i in range(len(Terminal)) :
        cell_index = i
        bottom_chip_ax.add_patch(
                patches.Rectangle(  #{'TA': [(7, 10), (14, 10)], 'TB': [(7, 15),  (16, 15)]}
                    (Terminal[cell_index][0],Terminal[cell_index][1]),
                    Terminal[cell_index][2],
                    Terminal[cell_index][3],
                    fill = True,
                    edgecolor = '#990',
                    facecolor = '#990',
                    alpha = 0.7 #0.3 original
                )
            )

    #for cell
    for i in  list(bottom_cell.keys()) :
        cell_index = i
        bottom_chip_ax.add_patch(
                patches.Rectangle( #{'TA': [(7, 10), (14, 10)], 'TB': [(7, 15),  (16, 15)]}
                    (initial_cell[cell_index][0],initial_cell[cell_index][1]),
                    initial_cell[cell_index][2],
                    initial_cell[cell_index][3],
                    fill = True,
                    edgecolor = '#333',
                    facecolor = '#777',
                    alpha = 0.5 #0.3 original
                )
            )

    #for row line
    for i in range(DieRows):
        bottom_chip_ax.add_patch(
        patches.Rectangle( 
                (Die_size[0], Die_size[1]+(DieRow_height*i) ),
                Die_width, 
                1,
                fill = True,
                edgecolor = '#900',
                facecolor = '#900',
                alpha = 0.5 #0.3 original
            )
        )
    #ax.autoscale_view()
    plt.savefig(system_arg[3]+"_bottom_partition.png")
    #plt.show()


    #----------------------------------------------plot top picture
    fig = plt.figure(figsize=png_size)
    plt.title("Partition Top Chip",fontsize="xx-large",fontweight="heavy")
    top_chip_ax = fig.add_subplot(111)
    top_chip_ax.set_xbound(Die_size[0],Die_size[2])
    top_chip_ax.set_ybound(Die_size[1],Die_size[3])

    #for Terminal
    for i in range(len(Terminal)) :
        cell_index = i
        top_chip_ax.add_patch(
                patches.Rectangle(  #{'TA': [(7, 10), (14, 10)], 'TB': [(7, 15),  (16, 15)]}
                    (Terminal[cell_index][0],Terminal[cell_index][1]),
                    Terminal[cell_index][2],
                    Terminal[cell_index][3],
                    fill = True,
                    edgecolor = '#990',
                    facecolor = '#990',
                    alpha = 0.7 #0.3 original
                )
            )
    #for cell
    for i in list(top_cell.keys()) :
        cell_index = i
        top_chip_ax.add_patch(
                patches.Rectangle(  #{'TA': [(7, 10), (14, 10)], 'TB': [(7, 15),  (16, 15)]}
                    (initial_cell[cell_index][0],initial_cell[cell_index][1]),
                    initial_cell[cell_index][2],
                    initial_cell[cell_index][3],
                    fill = True,
                    edgecolor = '#333',
                    facecolor = '#777',
                    alpha = 0.3 #0.3 original
                )
            )

    #for row line
    for i in range(DieRows):
        top_chip_ax.add_patch(
        patches.Rectangle( 
                (Die_size[0], Die_size[1]+(DieRow_height*i) ),
                Die_width, 
                1,
                fill = True,
                edgecolor = '#900',
                facecolor = '#900',
                alpha = 0.5 #0.3 original
            )
        )

    #ax.autoscale_view()
    plt.savefig(system_arg[3]+"_top_partition.png")
    #plt.show()













#----------------------------------------------plot draw3d
if(Draw_3D_Chip):

    fig = plt.figure(figsize=png_size)
    plt.title("3D Chip",fontsize="xx-large",fontweight="heavy")
    
    x = np.zeros(Cell_number)
    y = np.zeros(Cell_number)
    z = np.zeros(Cell_number)

    for cell_index in list(bottom_cell.keys()) :
        x[cell_index] = bottom_cell[cell_index][1]
        y[cell_index] = bottom_cell[cell_index][2]
        z[cell_index] = 0
    for cell_index in list(top_cell.keys()) :
        x[cell_index] = top_cell[cell_index][1]
        y[cell_index] = top_cell[cell_index][2]
        z[cell_index] = Die_size[3]/2

    fig = plt.figure()



    ax = Axes3D(fig)
    ax.set_xlim(0,Die_size[2])
    ax.set_ylim(0,Die_size[3])

    ax.scatter3D(x, y, z, c='red')

    #ax.autoscale_view()
    plt.savefig(system_arg[3]+"_3D.png")
    #plt.show()
