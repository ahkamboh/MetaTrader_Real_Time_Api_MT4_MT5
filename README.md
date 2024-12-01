# MetaTrader Real-Time API (MT4 & MT5) Documentation

This repository provides a powerful, real-time data retrieval solution for MetaTrader 4 (MT4) and MetaTrader 5 (MT5) trading platforms. By leveraging this API, you can fetch live market data and display it on your website or application.

---

## Features
- Real-time data retrieval from MT4/MT5.
- Easy integration with websites or applications.
- Secure and scalable setup.
- Compatible with RDP for enhanced security.

---

## Prerequisites
Before setting up the API, ensure you have the following:
1. **MetaTrader Platform** (MT4 or MT5) installed on your RDP.
2. **Remote Desktop Access** to your server (RDP).

---

## Setup Instructions

#### **1. EA Script Setup**
This section guides you on how to configure and deploy the provided EA script to MetaTrader (MT4/MT5).

##### **Prerequisites**
1. A MetaTrader terminal (MT4 or MT5).
2. Web server API endpoint ready to receive data (`API_URL`).
3. Ensure necessary URLs are allowed in MetaTrader:
   - Go to `Tools -> Options -> Expert Advisors`.
   - Add your server URLs, e.g., `https://your-site-url.com`.

##### **Steps to Setup the EA Script**
1. **Download the Script**:
   - Save the EA script file with a `.mq4` or `.mq5` extension depending on your MetaTrader version.
   
2. **Place the Script**:
   - Copy the script to the `Experts` directory of your MetaTrader:
     - For Windows: `MetaTrader/Experts/`
     - For Mac: `Applications/MetaTrader/Experts/`

3. **Update Configuration**:
   - Replace placeholders in the script:
     - `YOUR SITE NAME` → Your site or organization name.
     - `Your Site URL` → Base URL for your web server.
     - `Your API URL` → Endpoint for receiving data, e.g., `https://api.your-site-url.com/endpoint`.

4. **Compile the Script**:
   - Open the MetaEditor.
   - Load the script and click `Compile` to generate the `.ex4` or `.ex5` file.

5. **Attach to Chart**:
   - Restart MetaTrader.
   - Drag the EA script to the chart for the account you wish to monitor.

---

#### **2. Backend Integration**
This backend script is designed to store and manage account data sent from the EA.

##### **Prerequisites**
1. Node.js environment with `Next.js` and `MySQL`.
2. Database credentials configured in your project.

##### **Database Schema**
Ensure your database has a table called `mt_accounts` with the following fields:

```sql
CREATE TABLE mt_accounts (
  id INT AUTO_INCREMENT PRIMARY KEY,
  mt_login VARCHAR(50) NOT NULL,
  platform_password VARCHAR(255),
  account_number VARCHAR(50),
  current_balance DECIMAL(10, 2),
  current_equity DECIMAL(10, 2),
  floating_pl DECIMAL(10, 2),
  margin_level DECIMAL(10, 2),
  broker VARCHAR(100),
  server VARCHAR(100),
  updated_at DATETIME,
  is_demo BOOLEAN,
  balance_history JSON,
  equity_history JSON
);
```

##### **Deploy Backend Code**
1. Copy the provided backend function into your `Next.js` project under `/pages/api/mt-data.ts`.
2. Update the `pool` configuration in `db.ts` to match your MySQL credentials.
3. Deploy your application to a production server (e.g., Vercel, AWS).

##### **Configure Security**
1. Restrict CORS to only allow requests from trusted origins.
2. Validate incoming data to prevent unauthorized access.

---

#### **3. Testing**
1. Attach the EA script to a demo account in MetaTrader.
2. Monitor logs in MetaTrader (`Experts` tab) for successful API requests.
3. Check your backend logs or database to verify data storage.

---

Feel free to reach out if you need further assistance or encounter issues.
