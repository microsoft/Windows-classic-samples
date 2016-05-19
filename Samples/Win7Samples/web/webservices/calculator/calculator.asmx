<%@ WebService Language="C#" Class="CalculatorService" %>

using System;
using System.Web.Services;
/*************************************************************************/
/* Class: CalculatorService                                              */
/*************************************************************************/
public class CalculatorService : WebService{
    private static string sMemoryValue = "0";   // String variable to store the results in memory.
    private static string sTemp = "0";          // Temporary string.
    private static bool bHasPeriod = false;
    private static string sResult = "0.";       // The running results of the calculations.
    private static string sOperation = "+";     // Default operation for the calculator.
    private static double dNumberOne = 0.0;     // Variable one used for calculation.
    private static double dNumberTwo = 0.0;     // Variable two used for calculation.
    private static bool bNewValue = true;       // Flag for a new value.
/*************************************************************************/
/* Function createNum(string)                                            */
/* Purpose: Create number string from user's input .                     */
/* Output: None                                                          */
/*************************************************************************/
    private void createNum(string s){
        if (bNewValue){              // Is the input from the user a new value?
            bHasPeriod = false;      // New number can contain a decimal point.
            sResult = s;             // Assign new value to the string.
        }else{
            sResult = sResult + s;   // Append user input to the string.
        }
        bNewValue = false;
    }
/*************************************************************************/
/* Function: Clear()                                                     */
/* Purpose: Set all variables back to their original state or value.     */
/* Output: None                                                          */
/*************************************************************************/    
    private void Clear(){
        // Clear all variables and set to original values.
        sResult= "0.";
        dNumberOne   = 0;
        dNumberTwo   = 0;
        sOperation = "+";
        bNewValue = true;
    }
/*************************************************************************/
/* Function: doOperation()                                               */
/* Purpose: Perform the operation chosen by the user.                    */
/* Output: None. Returns a string that contains the result from the      */
/*      calculation.                                                     */ 
/*************************************************************************/
    private string doOperation(string sOp){
        // Parses sResult string into type double and assigns to dNumberTwo.
        dNumberTwo = double.Parse(sResult);
        switch(sOperation){
            case "+":
                sResult = (dNumberOne + dNumberTwo).ToString();
                break;
            case "-":
                sResult = (dNumberOne - dNumberTwo).ToString();
                break;
            case "*":
                sResult = (dNumberOne * dNumberTwo).ToString();
                break;
            case "/":
                // Cannot divide by zero.
                if(dNumberTwo == 0){
                    Clear(); // Clear all values.
                    return "Cannot divide by zero.";
                }else{
                    // Perform the calculation.
                    sResult = (dNumberOne / dNumberTwo).ToString();
                }
                break;
        }
        // Has the user chosen the equal sign?
        if (sOp != "equals"){
            sOperation = sOp;
            dNumberOne = double.Parse(sResult);
        }else{
            bHasPeriod = false;
            dNumberOne = 0.0;
            sOperation = "+";
        }
        bNewValue = true;
        bHasPeriod = false;
        return sResult;
    }
/******************* WebMethod *******************************************/
/* Function doCalc(string)                                               */
/* Purpose: Perform calculation from user input.                         */ 
/* Output: The result (sResult) value from the input.                    */
/*************************************************************************/
	[ WebMethod ] 
	public string doCalc(string sBtnValue)
	{   
        dNumberTwo = double.Parse(sResult);

        switch(sBtnValue){
            case ".":
                if(!bHasPeriod){ // A number can only have one decimal point.
                    if(!bNewValue) 
                        sResult += ".";
                    else{ 
                        sResult = "0.";
                        bNewValue = false;
                    }
                    bHasPeriod = true;
                    return sResult;
                }else{
                    return sResult;
                }   
        
            case "1": case "2": case "3": case "4": case "5": case "6": case "7": case "8": case "9": case "0":
                createNum(sBtnValue);
                return sResult;
                
            case "+": case "-": case "*": case "/": case "equals":
                return doOperation(sBtnValue);

            case "sqrt":
                // Square the value.
                sResult = (Math.Sqrt(double.Parse(sResult))).ToString();
                bNewValue = true;
                return sResult;                

            case "1/x":
                // Invert the value.
                sResult = (1 / (double.Parse(sResult))).ToString();
                bNewValue = true;
                return sResult;

            case "divBy100":
                // What is the percent value?
                sResult = (double.Parse(sResult) / 100).ToString();
                bNewValue = true;
                return sResult;

            case "+/-":
                sResult = (double.Parse(sResult) * -1).ToString();
                return sResult;
                
            case "C":
                // Clear all values.
                Clear();
                return sResult;

            case "BackSpace":
                sTemp = "";
                for(int i = 0; i < sResult.Length - 1; i++){
                    sTemp += sResult[i];
                }
                // Length of sTemp is 0?
                if(sTemp.Length == 0){
                    //Set sResult to 0 and set bNewValue flag to true.
                    sResult = "0";
                    bNewValue = true;
                // Assign sResult to new value.
                }else{
                    sResult = sTemp;
                }            
                return sResult;  
            
            case "CE":
                sResult = "0";
                bNewValue = true;
                return "0";

            case "MS":
                // Save value to memory.
                sMemoryValue = sResult;
                bNewValue = true;
                return sResult;
                                
            case "M+":
                // Add to memory the value from the user.
                sMemoryValue = (double.Parse(sResult) + double.Parse(sMemoryValue)).ToString();
                bNewValue = true;
                return sResult;                                          

            case "MR":
                // Recall the memory value and return it to the callback.
                bNewValue = true;
                sResult = sMemoryValue;
                return sMemoryValue;
            
            case "MC":
                // Clear the memory value.
                sMemoryValue = "0.";
                bNewValue = true;
                return sResult;                            
                
            case "load":
                // Set all variables to default values.
                Clear();
                sMemoryValue = "0";     
                return sResult;                       
            
            default:
                return sResult;
        }
	}
/******************* WebMethod *******************************************/
/* Function doPaste(string)                                              */
/* Purpose: Perform paste.                                               */ 
/* Output: The result (sResult) value from the input.                    */
/*************************************************************************/
	[ WebMethod ] 
	public string doPaste(string x)
	{   
        bNewValue = false;
        dNumberTwo = double.Parse(x);
        sResult = x;
        return sResult;
    }    
}


