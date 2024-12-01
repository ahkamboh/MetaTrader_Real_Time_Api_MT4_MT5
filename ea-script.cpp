#property copyright "YOUR SITE NAME"
#property link      "Your Site URL"
#property version   "1.00"
#property strict

// Constants - Updated with your production URL
#define API_URL "Your API URL"

// Global variables
bool firstRun = true;
string currentLogin;
string userPassword = "";

// Custom function to show password input dialog
bool ShowPasswordDialog()
{
    long chart_id = ChartID();
    
    // Get chart dimensions
    int chartWidth = ChartGetInteger(chart_id, CHART_WIDTH_IN_PIXELS);
    int chartHeight = ChartGetInteger(chart_id, CHART_HEIGHT_IN_PIXELS);
    
    // Calculate center position
    int dialogWidth = 300;
    int dialogHeight = 120;
    int xPos = (chartWidth - dialogWidth) / 2;
    int yPos = (chartHeight - dialogHeight) / 2;
    
    // Create objects for password input dialog - Centered on chart
    ObjectCreate(chart_id, "passwordBg", OBJ_RECTANGLE_LABEL, 0, 0, 0);
    ObjectSetInteger(chart_id, "passwordBg", OBJPROP_XDISTANCE, xPos);
    ObjectSetInteger(chart_id, "passwordBg", OBJPROP_YDISTANCE, yPos);
    ObjectSetInteger(chart_id, "passwordBg", OBJPROP_XSIZE, dialogWidth);
    ObjectSetInteger(chart_id, "passwordBg", OBJPROP_YSIZE, dialogHeight);
    ObjectSetInteger(chart_id, "passwordBg", OBJPROP_BGCOLOR, clrWhite);
    ObjectSetInteger(chart_id, "passwordBg", OBJPROP_BORDER_TYPE, BORDER_FLAT);
    ObjectSetInteger(chart_id, "passwordBg", OBJPROP_CORNER, CORNER_LEFT_UPPER);
    
    // Label - Centered
    ObjectCreate(chart_id, "passwordLabel", OBJ_LABEL, 0, 0, 0);
    ObjectSetString(chart_id, "passwordLabel", OBJPROP_TEXT, "Enter Platform Password:");
    ObjectSetInteger(chart_id, "passwordLabel", OBJPROP_XDISTANCE, xPos + 20);
    ObjectSetInteger(chart_id, "passwordLabel", OBJPROP_YDISTANCE, yPos + 20);
    ObjectSetInteger(chart_id, "passwordLabel", OBJPROP_COLOR, clrBlack);
    
    // Edit box for password - Centered
    ObjectCreate(chart_id, "passwordEdit", OBJ_EDIT, 0, 0, 0);
    ObjectSetInteger(chart_id, "passwordEdit", OBJPROP_XDISTANCE, xPos + 20);
    ObjectSetInteger(chart_id, "passwordEdit", OBJPROP_YDISTANCE, yPos + 50);
    ObjectSetInteger(chart_id, "passwordEdit", OBJPROP_XSIZE, 260);
    ObjectSetInteger(chart_id, "passwordEdit", OBJPROP_BGCOLOR, clrWhite);
    ObjectSetInteger(chart_id, "passwordEdit", OBJPROP_BORDER_COLOR, clrGray);
    ObjectSetString(chart_id, "passwordEdit", OBJPROP_TEXT, "");
    
    // Button - Centered
    ObjectCreate(chart_id, "passwordButton", OBJ_BUTTON, 0, 0, 0);
    ObjectSetInteger(chart_id, "passwordButton", OBJPROP_XDISTANCE, xPos + 20);
    ObjectSetInteger(chart_id, "passwordButton", OBJPROP_YDISTANCE, yPos + 80);
    ObjectSetInteger(chart_id, "passwordButton", OBJPROP_XSIZE, 260);
    ObjectSetInteger(chart_id, "passwordButton", OBJPROP_BGCOLOR, clrDodgerBlue);
    ObjectSetInteger(chart_id, "passwordButton", OBJPROP_COLOR, clrWhite);
    ObjectSetString(chart_id, "passwordButton", OBJPROP_TEXT, "Submit");
    
    // Wait for user input
    while(!IsStopped())
    {
        if(ObjectGetInteger(chart_id, "passwordButton", OBJPROP_STATE))
        {
            userPassword = ObjectGetString(chart_id, "passwordEdit", OBJPROP_TEXT);
            
            // Clean up objects
            ObjectDelete(chart_id, "passwordBg");
            ObjectDelete(chart_id, "passwordLabel");
            ObjectDelete(chart_id, "passwordEdit");
            ObjectDelete(chart_id, "passwordButton");
            
            return true;
        }
        Sleep(100);
    }
    
    return false;
}

//+------------------------------------------------------------------+
//| Expert initialization function                                     |
//+------------------------------------------------------------------+
int OnInit()
{
    // Automatically get account login
    currentLogin = IntegerToString(AccountInfoInteger(ACCOUNT_LOGIN));
    
    // Validate login
    if(StringLen(currentLogin) == 0) {
        Print("Error: Could not get account login number");
        return INIT_FAILED;
    }
    
    // Show password dialog
    if(!ShowPasswordDialog())
    {
        Print("Password input cancelled");
        return INIT_FAILED;
    }
    
    Print("EA Initialized for account: ", currentLogin);
    
    // Send initial data when EA is first attached
    if(firstRun) {
        SendAccountData();
        firstRun = false;
    }
    
    return(INIT_SUCCEEDED);
}

//+------------------------------------------------------------------+
//| Expert tick function                                              |
//+------------------------------------------------------------------+
void OnTick()
{
    static datetime lastSend = 0;
    datetime currentTime = TimeCurrent();
    
    // Send data every 2 seconds
    if(currentTime - lastSend >= 2) 
    {
        SendAccountData();
        lastSend = currentTime;
    }
}

//+------------------------------------------------------------------+
//| Function to send account data to server                           |
//+------------------------------------------------------------------+
void SendAccountData()
{
    // Get account info
    double balance = AccountInfoDouble(ACCOUNT_BALANCE);
    double equity = AccountInfoDouble(ACCOUNT_EQUITY);
    double floating_pl = AccountInfoDouble(ACCOUNT_PROFIT);
    double margin_level = AccountInfoDouble(ACCOUNT_MARGIN_LEVEL);
    
    // Calculate margin level if it's not available
    if(margin_level == 0 || margin_level == EMPTY_VALUE) {
        double margin = AccountInfoDouble(ACCOUNT_MARGIN);
        if(margin > 0) margin_level = equity / margin * 100;
    }
    
    // Prepare headers with additional security headers
    string headers = "Content-Type: application/json\r\n";
    headers += "Connection: keep-alive\r\n";
    headers += "Origin: Your Site URL\r\n";
    // " [Your EA Script file name]" just remove this and paste your EA Script file name without an extension like .mt4/mt5
    headers += "User-Agent: [Your EA Script file name]\r\n";
    
    // Create JSON payload with user password
    string postData = StringFormat(
        "{\"mt_login\":\"%s\",\"platform_password\":\"%s\",\"current_balance\":%.2f,\"current_equity\":%.2f,\"floating_pl\":%.2f,\"margin_level\":%.2f,\"account_type\":\"%s\",\"broker\":\"%s\",\"server\":\"%s\"}",
        currentLogin,
        userPassword,
        balance,
        equity,
        floating_pl,
        margin_level,
        AccountInfoInteger(ACCOUNT_TRADE_MODE) == ACCOUNT_TRADE_MODE_DEMO ? "DEMO" : "LIVE",
        AccountInfoString(ACCOUNT_COMPANY),
        AccountInfoString(ACCOUNT_SERVER)
    );
    
    // Debug info - Always show this for troubleshooting
    Print("Attempting to send data to: ", API_URL);
    Print("Data: ", postData);
    
    // Prepare arrays for WebRequest
    char post[], result[];
    ArrayResize(post, StringToCharArray(postData, post, 0, WHOLE_ARRAY, CP_UTF8)-1);
    
    // Send request to server with retry
    int maxRetries = 3;
    int currentTry = 0;
    bool success = false;
    
    while(!success && currentTry < maxRetries)
    {
        int res = WebRequest(
            "POST",
            API_URL,
            headers,
            5000,  // timeout
            post,
            result,
            headers
        );
        
        if(res != -1)
        {
            success = true;
            string resultStr = CharArrayToString(result);
            Print("Server response: ", resultStr);
        }
        else
        {
            currentTry++;
            int error = GetLastError();
            Print("Attempt ", currentTry, " failed with error: ", error);
            
            if(currentTry == 1)  // Show these messages only on first error
            {
                switch(error)
                {
                    case 4060:
                        MessageBox(StringFormat("Please add these URLs in Tools -> Options -> Expert Advisors:\n%s\n YOUR SITE URL", API_URL), "URL Configuration Required");
                        break;
                    case 4014:
                        MessageBox("Please check:\n1. Internet connection\n2. Firewall settings\n3. Antivirus blocking", "Connection Error");
                        break;
                    case 4015:
                        MessageBox("API endpoint not found. Please contact support.", "Server Error");
                        break;
                    case 4018:
                        MessageBox("Request timeout. Server took too long to respond.", "Timeout Error");
                        break;
                    case 4077:
                        MessageBox("HTTPS connection failed. Please check your internet security settings.", "Connection Error");
                        break;
                    default:
                        MessageBox(StringFormat("Error %d occurred. Please contact support.", error), "Error");
                        break;
                }
            }
            
            if(currentTry < maxRetries) {
                Print("Retrying in 1 second...");
                Sleep(1000);  // Wait 1 second before retry
            }
        }
    }
    
    if(!success) {
        Print("Failed to send data after ", maxRetries, " attempts");
    }
}

//+------------------------------------------------------------------+
//| Expert deinitialization function                                  |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
    string reasonText;
    switch(reason)
    {
        case REASON_PROGRAM:     reasonText = "Program"; break;
        case REASON_REMOVE:      reasonText = "Removed from chart"; break;
        case REASON_RECOMPILE:   reasonText = "Recompiled"; break;
        case REASON_CHARTCHANGE: reasonText = "Chart changed"; break;
        case REASON_CHARTCLOSE:  reasonText = "Chart closed"; break;
        case REASON_PARAMETERS:  reasonText = "Parameters changed"; break;
        case REASON_ACCOUNT:     reasonText = "Account changed"; break;
        default:                 reasonText = "Other reason"; break;
    }
    
    // Clean up any remaining dialog objects if they exist
    ObjectDelete(ChartID(), "passwordBg");
    ObjectDelete(ChartID(), "passwordLabel");
    ObjectDelete(ChartID(), "passwordEdit");
    ObjectDelete(ChartID(), "passwordButton");
    
    Print("EA stopped. Reason: ", reasonText, " (", reason, ")");
}
