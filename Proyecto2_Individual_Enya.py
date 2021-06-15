import numpy as np
import re

class Testbench:
    def __init__(self): #ACA
        # Enter the .sv file:
        self.FSM_name = ""
        self.name = ""
        self.format_name = ""
        #-------------Regular Expresions:---------------------
        self.reg_exp = {
            "module_name_str": "name.=.(.+?(?=\r))",
            "clock_name_str": "clock.=.(.+?(?=\r))",
            "reset_name_str": "reset.=.(.+?(?=\r))",
            "inputs_str": "inputs.=.\((.+)\)",
            "inputs_name_str": "(\w+)\[(\d+)\]",
            "outputs_str": "outputs.=.\((.+)\)",
            "outputs_name_str": "(\w+)\[(\d+)\]",
            "transitions_str":".(\w+).(\w+)\((.+?(?=\)))",
        } 
        #-------------FSM items items---------------------
        self.items  = {
            "module_name":"",
            "clock_name": "",
            "reset_name":"",
            "inputs":[],
            "outputs":[],
            "PS":[],
            "NS":[],
            "trans_conditions": [],
            "outputs_length": [],
            "inputs_length": [],
        }
    def get_string(self,fileName,pattern_case,reg_exp):
            archivo = open(fileName,'r')
            temp = []
            for line in archivo:
                m = re.search(reg_exp,line)
                if(m!=None):
                    if(pattern_case==0):
                        return str(m.group(1)).split(",")
                    elif(pattern_case==1):
                        temp.append(m.group(1))
                    elif(pattern_case ==2):
                        temp.append(m.group(2))
                    elif(pattern_case ==3):
                        temp.append(m.group(3))
            archivo.close()
            return temp 

    def get_io_names(self,i_list,reg_ex):
            names_temp = []
            size_temp = []
            
            for item in i_list:
                m = re.search(reg_ex,item)
                if(m!=None):
                    names_temp.append(m.group(1))
                    size_temp.append(m.group(2))
                else:
                    names_temp.append(item)
                    size_temp.append("1")
            return names_temp,size_temp


    def append_data(self):
            self.items["module_name"] = self.get_string(self.format_name,0,self.reg_exp["module_name_str"])
            self.items["clock_name"] = self.get_string(self.format_name,0,self.reg_exp["clock_name_str"])
            self.items["reset_name"] = self.get_string(self.format_name,0,self.reg_exp["reset_name_str"])
            self.items["inputs"] = self.get_string(self.format_name,0,self.reg_exp["inputs_str"])
            self.items["inputs"],self.items["inputs_length"] = self.get_io_names(self.items["inputs"],self.reg_exp["inputs_name_str"])
            
            self.items["outputs"] = self.get_string(self.format_name,0,self.reg_exp["outputs_str"])
            self.items["outputs"],self.items["outputs_length"] = self.get_io_names(self.items["outputs"],self.reg_exp["outputs_name_str"])
            self.items["PS"] = self.get_string(self.format_name,1,self.reg_exp["transitions_str"])
            self.items["NS"] = self.get_string(self.format_name,2,self.reg_exp["transitions_str"])
            self.items["trans_conditions"] = self.get_string(self.format_name,3,self.reg_exp["transitions_str"])
   
    def write_inputs(self,file,inouts,inouts_length):        
        for i in range(0,len(inouts)):
            file.write("   reg ")
            if(int(inouts_length[i]) > 1):
                file.write("[" + str(int(inouts_length[i])-1) +":0] ")
            file.write(inouts[i] + ";\r\n")
    def write_outputs(self,file,inouts,inouts_length):        
        for i in range(0,len(inouts)):
            file.write("   wire ")
            if(int(inouts_length[i]) > 1):
                file.write("[" + str(int(inouts_length[i])-1) +":0] ")
            file.write(inouts[i] + ";\r\n")
    
    def write_TB(self):
        clk_cycle = input("Clock cycle duration (Recommended: #1):\r\n")
        test_cycle = input("Tests cycle duration:\r\n")

        # Ask the user if the testbench need includes
        includes = []
        if_includes = input("Does the testbench need to include some files?\r\n 0.- No\r\n 1.- Yes \r\n")
        if(if_includes==1):
            add=1
            while(add):
                add_file=raw_input("Enter the name of the file to include with it's extension:\r\n")
                includes.append(add_file)
                add = input("Add another file?\r\n 0.- No\r\n 1.- Yes \r\n")
        #---Tesbench file name----------#
        tb_name = self.FSM_name+"_TB"

        #Writing includes in the testbench
        testbench = open(tb_name+".sv",'w')
        if(if_includes==1):
            for include in includes:
                testbench.write("`include \"" + include + "\"\r\n")
            testbench.write("\r\n")

        #Writing the testbench file:
        testbench.write("module " + tb_name + ";\r\n") 
        testbench.close()

        #Writing Inputs
        testbench = open(tb_name+".sv",'a') 
        testbench.write("   reg "+ self.items["clock_name"][0] +";\r\n")
        testbench.write("   reg "+ self.items["reset_name"][0]+ ";\r\n")
        self.write_inputs(testbench,self.items["inputs"],self.items["inputs_length"])
        #Writing Outputs
        self.write_outputs(testbench,self.items["outputs"],self.items["outputs_length"])

        #Top Module Instance Writing
        #Inputs
        testbench = open(tb_name+".sv",'a') 
        testbench.write("   "+self.FSM_name + " UUT(")
        testbench.write(self.items["clock_name"][0]+", "+ self.items["reset_name"][0] + ",")
        for i in range(0,np.shape(tb.items["inputs"])[0]):
            testbench = open(tb_name+".sv",'a')
            testbench.write(tb.items["inputs"][i]+',')

        #outputs
        for i in range(0,np.shape(tb.items["outputs"])[0]):
            testbench = open(tb_name+".sv",'a')
            if(i!=((np.shape(tb.items["outputs"])[0])-1)):
                testbench.write(tb.items["outputs"][i]+",")
            else:
                testbench.write(tb.items["outputs"][i])
        testbench.write(");\r\n")

        #Wite the stimulus for clk 
        testbench.write("\r\n   always #"+ str(clk_cycle) +" "+ self.items["clock_name"][0]+ " = ~" + self.items["clock_name"][0] + ";\r\n")
        #Stimulus test:
        testbench.write("\r\n   initial begin \r\n" +
                        "      $dumpfile(\"" + self.FSM_name+".vcd\");\r\n" +
                        "      $dumpvars(0," + tb_name + "); \r\n      ")

        testbench.write("/*-----Initialize clock and stimulate reset:----- */")
        testbench.write("\r\n      "+self.items["clock_name"][0]+"=0;")
        testbench.write("\r\n      "+self.items["reset_name"][0]+"=0;")
        testbench.write("\r\n      #1")
        testbench.write("\r\n      "+self.items["reset_name"][0]+"=1;")
        testbench.write("\r\n      #1")
        testbench.write("\r\n      "+self.items["reset_name"][0]+"=0;")
        testbench.write("\r\n      #1\r\n")
        testbench.write("/*--------------Start input tests---------------- */\r\n")

        for i in range(0,len(self.items["trans_conditions"])):
            if(self.items["trans_conditions"][i] != "-"):
                self.items["trans_conditions"][i]=re.sub("\&|\||\^|\<|\<\=|\>|\>=","*",self.items["trans_conditions"][i])
                self.items["trans_conditions"][i]=self.items["trans_conditions"][i].replace(" ","")
                self.items["trans_conditions"][i]=self.items["trans_conditions"][i].replace("**","*")
                self.items["trans_conditions"][i]=self.items["trans_conditions"][i].split("*")

                for j in range(0,len(self.items["trans_conditions"][i])):
                    testbench.write("\r\n/*---From " + self.items["PS"][i] + " to " +self.items["NS"][i] + "---------*/")
                    testbench.write("\r\n      " + self.items["trans_conditions"][i][j]+";")
                    testbench.write("\r\n      #"+str(test_cycle))

        testbench.write("\r\n      $finish;\r\n")
        testbench.write("   end\r\n")

        # Writing endmodule
        testbench.write("endmodule")
        testbench.close()
if __name__ == "__main__":
    tb = Testbench() 
    tb.FSM_name = raw_input("Write the name of the .sv file: ")
    tb.name = tb.FSM_name +".sv"
    tb.format_name = tb.FSM_name +".txt" 
    tb.append_data()
    tb.write_TB()

