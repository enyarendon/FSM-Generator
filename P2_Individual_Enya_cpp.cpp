#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include <regex>
#include <string> 
using namespace std;

string module_name_str = "name.=.(.+(?=\\r))";
string clock_name_str = "clock.=.(.+?(?=\\r))";
string reset_name_str = "reset.=.(.+?(?=\r))";
string inputs_str = "inputs.=.\\((.+)\\)";
string outputs_str = "outputs.=.\\((.+)\\)";
string transitions_str = ".(\\w+).(\\w+)\\((.+?(?=\\)))";
string outputs_name_str =  "(\\w+.)\\[(\\d+)\\]";
string inputs_name_str =  "(\\w+)\\[(\\d+)\\]";

regex module_name_re(module_name_str);
regex clock_name_re(clock_name_str);
regex reset_name_re(reset_name_str);
regex inputs_re (inputs_str);
regex outputs_re (outputs_str); 
regex transitions_re (transitions_str); 
regex outputs_name_re (outputs_name_str);
regex inputs_name_re (inputs_name_str);


class Testbench
{
  public:
    vector<string> module_name;
    vector<string> clock_name;
    vector<string> reset_name;
    vector<string> inputs;
    vector<int> inputs_length;
    vector<string> outputs;
    vector<int> outputs_length;
    vector<string> PS;
    vector<string> NS;
    vector<string> stim;    

    /* Files to read : */
    string design_name;
    string description_name;
    /* File to write: */
    ofstream tb_write;

    /*Testbench includes*/
    string includes[50];
    int if_includes;

    /* Program type (Sequential or combinational) */
    int clk_cycle;
    int test_cycle;

    Testbench(/* args */);
    vector<string> get_string(string, regex,int);

    vector<string> separate(vector<string>);
    void append_data(void);
    void get_io_names(void);
     vector<string> Replace_conditions(vector<string>,char,string);
    //Write file methods:
    void write_includes();
    void write_module_name();
    void write_inputs();
    void write_instantiation();
    void write_initial();
    void write_stimulus();
    void write_end();
    void ask_if_includes();
    void ask_cycles();
    
    //Find bits combinations
    vector<string> combinations;
    void findBitCombinations(int);
};

Testbench::Testbench(/* args */){}
vector<string> Testbench::get_string(string fileName, regex pattern, int pattern_case){   
    vector<string> temp;
    string line_temp;
    ifstream f;
    f.open(fileName);
    if (f.good()){
        while (!f.eof()){
            getline(f, line_temp);
            smatch matches;
            if (regex_search(line_temp, matches, pattern)){
                switch (pattern_case){
                case 0:
                    temp.push_back(matches[1].str());
                    break;
                case 1:
                    temp.push_back(matches[2].str());    //Group assignation of transitions                
                    break;
                case 2:
                    temp.push_back(matches[3].str());
                    break;
                }
            }
        }
        return temp;
    }
    return temp;
}

void Testbench::get_io_names(){
  // for (int i = 0 ; i < outputs.size() ; i++)
  //     cout << outputs[i]<<endl;
    for (int i = 0 ; i < inputs.size() ; i++){
        smatch matches;  
        if (regex_search(inputs[i], matches, inputs_name_re))
        {   int size = stoi(matches[2].str()); 
            inputs[i] = matches[1];
            inputs_length.push_back(size);
            //cout << "\nMATCHES[2]: " <<matches[2].str();         
        }
        else{
            inputs_length.push_back(1);
        }
    }  
    for (int i = 0 ; i < outputs.size() ; i++){
        smatch group;  
        if (regex_search(outputs[i], group, outputs_name_re))
        {   
            int size = stoi(group[2].str()); 
            outputs[i] = group[1];
            outputs_length.push_back(size);
        }
        else{
            outputs_length.push_back(1);
        }
    }  
}
vector<string> Testbench::separate(vector<string> names){
    string temp = names.at(0);
    vector<string> v_temp;
    stringstream ss(temp);
    while( ss.good() )
    {
        string substr;
        getline( ss, substr, ',' );
        v_temp.push_back( substr );
    }
    return v_temp;
}
void Testbench::append_data(){

    module_name = get_string(description_name, module_name_re,0);
    clock_name = get_string(description_name, clock_name_re,0);
    reset_name = get_string(description_name, reset_name_re,0);
    inputs = get_string(description_name, inputs_re,0);
    inputs = separate(inputs);

    outputs = get_string(description_name, outputs_re,0);
    outputs = separate(outputs);

    PS = get_string(description_name, transitions_re,0);
    NS = get_string(description_name, transitions_re,1);
    stim = get_string(description_name, transitions_re,2);
}

void Testbench::write_module_name(){
  tb_write << "module " + module_name[0] + "_TB;\r\n";
}

void Testbench::write_inputs(){

  tb_write << "  reg "+ clock_name[0] + ";\r\n";
  tb_write << "  reg "+ reset_name[0]+ ";\r\n";
    

   for(int i =0 ; i < inputs.size() ; i++){
    tb_write << ("  reg ");  
    if(inputs_length[i] > 1){
      tb_write << "["+ to_string(inputs_length[i]-1) + ":0] " << inputs[i] + ";\r\n";
    }else{
       tb_write << inputs[i] + ";\r\n";
    }
  }
  for(int i =0 ; i < outputs.size() ; i++){
    tb_write << ("  wire "); 
    if(outputs_length[i] > 1){
      tb_write << "["+ to_string(outputs_length[i]-1) + ":0] " << outputs[i] + ";\r\n";
    }else{
      tb_write << outputs[i] + ";\r\n";
    }
  }

}

void Testbench::write_instantiation(){
  tb_write<< "\r\n  " << module_name[0] + " UUT(";
   /* If sequential instantiate clk and rst */
  tb_write << clock_name[0] + "," + reset_name[0] +",";
  /* instantiate inputs */
  for(int i =0 ; i < inputs.size() ; i++){
      tb_write << inputs[i] + ",";
  }
  /* instantiate outputs */
   for(int i =0 ; i < outputs.size() ; i++){
    if(i == outputs.size()-1)
      tb_write << outputs[i];
    else
      tb_write << outputs[i] + ",";
  }
  /* close instantiation */
  tb_write << ");\r\n";

  /*-------------Write always clock------------*/
  tb_write << "\r\n   always #"+ to_string(clk_cycle) +" " + clock_name[0] + " = ~" + clock_name[0] +";\r\n";
}

void Testbench::write_initial(){
  tb_write << "\r\n initial begin \r\n";
  tb_write << "   $dumpfile(\"" + module_name[0] +".vcd\");\r\n" +
              "   $dumpvars(1," + module_name[0] + "_TB); \r\n";

  tb_write<< "//-----Initialize clock and stimulate reset:----- ";
  tb_write<<("\r\n      " + clock_name[0] + "=0;");
  tb_write<<("\r\n      " + reset_name[0] + "=0;");
  tb_write << ("\r\n      #1");
  tb_write << ("\r\n      " + reset_name[0] + "=1;");
  tb_write << ("\r\n      #1");
  tb_write << ("\r\n      " + reset_name[0] + "=0;");
  tb_write << ("\r\n      #1\r\n      ");
}


void Testbench::write_end(){
  tb_write << "\r\n     $finish; \r\n end \r\nendmodule";
}

bool replace(string& str, const string& from, const string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void Testbench::write_stimulus(){  
  /*--------------Remove &,|,^,<,etc from stimulus----------*/
  for (int i = 0 ; i < stim.size() ; i++){
    stim[i] = regex_replace(stim[i],regex("\\&|\\||\\^|\\<|\\<\\=|\\>|\\>="),"*");
    stim[i] = regex_replace(stim[i],regex("\\*\\*"),","); 
    stim[i] = regex_replace(stim[i],regex(" "),""); 
    
  }
  /*--------------Separate stimulus by commas----------*/
  vector<vector<string>> tb_stim;
  for (int j = 0 ; j < stim.size() ; j++){
      string temp = stim[j];
      vector<string> vs_temp;
      stringstream ss(temp);
      while( ss.good() )
      {
          string substr;
          getline( ss, substr, ',' );
          vs_temp.push_back( substr );
      }
      tb_stim.push_back(vs_temp);
    }

    for (int i = 0; i < tb_stim.size(); i++){
      for (int j = 0; j < tb_stim[i].size(); j++){
          if(tb_stim[i][j]!="-"){
            tb_write << "\r\n/*---From "<< PS[i] << " to " << NS[i] << "---------*/";
            tb_write << "\r\n      " + tb_stim[i][j]+";";
            tb_write << "\r\n      #" + to_string(test_cycle);
          }
      }
    }    
}

  void Testbench::ask_cycles(){
    cout << "Clock cycle duration (Recommended: #1):\r\n";
    cin >> clk_cycle;
    cout << "Tests cycle duration:\r\n";
    cin >> test_cycle;
  }
    
  void Testbench::ask_if_includes(){
    cout << "Does the testbench need to include some files?\r\n 0.- No\r\n 1.- Yes \r\n";
    cin >> if_includes;
    if(if_includes==1){
      int add=1;
      int i = 0;
      while(add){
        string add_file;
        cout << "Enter the name of the file to include with it's extension:\r\n";
        cin >> add_file;
        includes[i] = (add_file);
        i++;
        cout << "Add another file?\r\n 0.- No\r\n 1.- Yes \r\n";
        cin >> add;
      }
    }
  }

  void Testbench::write_includes(){
    if(if_includes==1){
      int i = 0 ;
      while(includes[i]!=""){
        tb_write << "`include \"" + includes[i] + "\"\r\n\r\n";
        i++;
      }
    }
  }

int main () {

  Testbench tb_file;
  tb_file.description_name= "FSM_Control.txt";
  tb_file.append_data();
  tb_file.get_io_names();
  //Ask cycles
  tb_file.ask_cycles();
  //Ask if includes
  tb_file.ask_if_includes();
  //Write to file
  tb_file.tb_write.open(tb_file.module_name[0]+"_TB.sv");
  tb_file.write_includes();
  tb_file.write_module_name();
  tb_file.write_inputs();
  tb_file.write_instantiation();
  tb_file.write_initial();
  
  // string x = "ALUSrcB[2]";
  // smatch o;
  // cout <<tb_file.outputs[1] <<endl;
  // vector<int> lens;
  // for(int i = 0; i < tb_file.outputs.size() ; i++){
  //   if(regex_search(tb_file.outputs[i],o,regex("(\\w+.)\\[(\\d+)\\]"))){
  //     cout << o[2]<< endl;
  //     lens.push_back(stoi(o[2]));
  //   }else
  //     lens.push_back(1);
  // }
  
  tb_file.write_stimulus();
  tb_file.write_end();
 return 0;
}

