import os,re
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import openpyxl


def findAllExcelFile(base,group_number):
    for root,ds,fs in os.walk(base):
        for f in fs:
            if re.search(".xls",f) and re.search("FeedbackGroup%d" %group_number, f):
                fullname =os.path.join(root,f)
                yield fullname


def createSpeedExcelFile():
    num_sheets = 6
    # Create an Excel writer
    speedFile = "C:\\Users\\wangr\\training\\multiPeopleTrainingTempo\\secExamResults\\secExam\\squatSpeeds.xlsx"
    writer = pd.ExcelWriter(speedFile, engine='xlsxwriter')
    # Create empty DataFrames and save them as sheets
    for sheet_number in range(num_sheets):
        sheet_name = f'Group{sheet_number + 1}'
        empty_df = pd.DataFrame()
        empty_df.to_excel(writer, sheet_name=sheet_name, index=False)
    # Save the Excel file
    writer.close()

if __name__ == '__main__':
    #createSpeedExcelFile()
    speedFile = "C:\\Users\\wangr\\training\\multiPeopleTrainingTempo\\secExamResults\\secExam\\squatSpeeds.xlsx"
    base = "C:\\Users\\wangr\\training\\multiPeopleTrainingTempo\\secExamResults\\secExam"
    with pd.ExcelWriter(speedFile, engine='openpyxl') as writer:
        fig, axes = plt.subplots(nrows=6, ncols=3)
        for n in [1,2,3,4,5,6]:
            sheet_name = f'Group{n}'
            empty_df = pd.DataFrame()
            empty_df.to_excel(writer, sheet_name=sheet_name, index=False)
            speedSheet = writer.sheets["Group" + str(n)]
            for i in findAllExcelFile(base,n):
                print(i)
                df = pd.read_excel(i,sheet_name="Ave")
                if(len(df.columns) == 6):
                    names = ["times", "P1", "P2", "P3", "space1", "Ave"]
                if(len(df.columns) == 8):
                    names = ["times", "P1", "P2", "P3", "space1", "Ave", "space2","space3"]
                df.columns = names
                df_P1 = df[["times", "P1"]].drop_duplicates(subset="P1")[df["P1"] > 0]
                df_P2 = df[["times", "P2"]].drop_duplicates(subset="P2")[df["P2"] > 0]
                df_P3 = df[["times", "P3"]].drop_duplicates(subset="P3")[df["P3"] > 0]
                if re.search("no", i):
                    axes[n-1,0].plot(df_P1["times"], df_P1["P1"], label = "P1")
                    axes[n-1,0].plot(df_P2["times"], df_P2["P2"], label = "P2")
                    axes[n-1,0].plot(df_P3["times"], df_P3["P3"], label = "P3")
                    axes[n-1, 0].set_xlim(0,45000)
                    axes[n-1, 0].set_ylim(1500, 4000)
                    column_index = list(range(6))
                    column_letter = [chr(65 + index) for index in column_index]
                    for index in range(df_P1.shape[0]):
                        row = df_P1.iloc[index]
                        speedSheet[column_letter[0] + str(index + 2)] = row[0]
                        speedSheet[column_letter[1] + str(index + 2)] = row[1]
                    for index in range(df_P2.shape[0]):
                        row = df_P2.iloc[index]
                        speedSheet[column_letter[2] + str(index + 2)] = row[0]
                        speedSheet[column_letter[3] + str(index + 2)] = row[1]
                    for index in range(df_P3.shape[0]):
                        row = df_P3.iloc[index]
                        speedSheet[column_letter[4] + str(index + 2)] = row[0]
                        speedSheet[column_letter[5] + str(index + 2)] = row[1]

                if re.search("fix", i):
                    axes[n-1,1].plot(df_P1["times"], df_P1["P1"], label = "P1")
                    axes[n-1,1].plot(df_P2["times"], df_P2["P2"], label = "P2")
                    axes[n-1,1].plot(df_P3["times"], df_P3["P3"], label = "P3")
                    axes[n-1, 1].set_xlim(0,45000)
                    axes[n-1, 1].set_ylim(1500, 4000)
                    column_index = list(range(6,12))
                    column_letter = [chr(65 + index) for index in column_index]
                    for index in range(df_P1.shape[0]):
                        row = df_P1.iloc[index]
                        speedSheet[column_letter[0] + str(index + 2)] = row[0]
                        speedSheet[column_letter[1] + str(index + 2)] = row[1]
                    for index in range(df_P2.shape[0]):
                        row = df_P2.iloc[index]
                        speedSheet[column_letter[2] + str(index + 2)] = row[0]
                        speedSheet[column_letter[3] + str(index + 2)] = row[1]
                    for index in range(df_P3.shape[0]):
                        row = df_P3.iloc[index]
                        speedSheet[column_letter[4] + str(index + 2)] = row[0]
                        speedSheet[column_letter[5] + str(index + 2)] = row[1]

                if re.search("realtime", i):
                    axes[n-1,2].plot(df_P1["times"], df_P1["P1"], label = "P1")
                    axes[n-1,2].plot(df_P2["times"], df_P2["P2"], label = "P2")
                    axes[n-1,2].plot(df_P3["times"], df_P3["P3"], label = "P3")
                    axes[n-1, 2].set_xlim(0,45000)
                    axes[n-1, 2].set_ylim(1500, 4000)
                    axes[n - 1, 2].legend()
                    column_index = list(range(12,18))
                    column_letter = [chr(65 + index) for index in column_index]
                    for index in range(df_P1.shape[0]):
                        row = df_P1.iloc[index]
                        speedSheet[column_letter[0] + str(index + 2)] = row[0]
                        speedSheet[column_letter[1] + str(index + 2)] = row[1]
                    for index in range(df_P2.shape[0]):
                        row = df_P2.iloc[index]
                        speedSheet[column_letter[2] + str(index + 2)] = row[0]
                        speedSheet[column_letter[3] + str(index + 2)] = row[1]
                    for index in range(df_P3.shape[0]):
                        row = df_P3.iloc[index]
                        speedSheet[column_letter[4] + str(index + 2)] = row[0]
                        speedSheet[column_letter[5] + str(index + 2)] = row[1]
    plt.show()










