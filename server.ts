import { NextRequest, NextResponse } from 'next/server';
import pool from '@/lib/db';
import { format } from 'date-fns';

// Store MT data and link it to user
async function storeMTData(data: any) {
  const connection = await pool.getConnection();
  try {
    const now = format(new Date(), 'yyyy-MM-dd HH:mm:ss');
    
    // First, get existing data
    const [rows] = await connection.query(
      'SELECT balance_history, equity_history FROM mt_accounts WHERE mt_login = ?',
      [data.mt_login]
    );
    const existingUser = rows as any[];

    if (existingUser.length === 0) {
      // New user - Insert with initial arrays
      const [result] = await connection.query(
        `INSERT INTO mt_accounts 
         SET mt_login = ?,
             platform_password = ?,
             account_number = ?,
             current_balance = ?,
             current_equity = ?,
             floating_pl = ?,
             margin_level = ?,
             broker = ?,
             server = ?,
             updated_at = ?,
             is_demo = ?,
             balance_history = ?,
             equity_history = ?`,
        [
          data.mt_login,
          data.platform_password,
          data.mt_login,
          data.current_balance,
          data.current_equity,
          data.floating_pl,
          data.margin_level,
          data.broker,
          data.server,
          now,
          data.account_type === 'DEMO' ? 1 : 0,
          JSON.stringify([Number(data.current_balance)]),
          JSON.stringify([Number(data.current_equity)])
        ]
      );
      return { ...result, isNewUser: true };
    } else {
      // Get existing histories
      let balanceHistory = [];
      let equityHistory = [];
      
      try {
        balanceHistory = JSON.parse(existingUser[0].balance_history || '[]');
        equityHistory = JSON.parse(existingUser[0].equity_history || '[]');
      } catch (e) {
        console.error('Error parsing existing histories:', e);
      }

      // Ensure they're arrays
      if (!Array.isArray(balanceHistory)) balanceHistory = [];
      if (!Array.isArray(equityHistory)) equityHistory = [];

      // Add new values
      balanceHistory.push(Number(data.current_balance));
      equityHistory.push(Number(data.current_equity));

      // Keep only last 20 values
      balanceHistory = balanceHistory.slice(-20);
      equityHistory = equityHistory.slice(-20);

      console.log('New balance history:', balanceHistory);
      console.log('New equity history:', equityHistory);

      // Update using JSON_ARRAY
      const [result] = await connection.query(
        `UPDATE mt_accounts 
         SET current_balance = ?,
             current_equity = ?,
             floating_pl = ?,
             margin_level = ?,
             broker = ?,
             server = ?,
             platform_password = ?,
             is_demo = ?,
             updated_at = ?,
             balance_history = JSON_ARRAY_APPEND(
               COALESCE(
                 CASE 
                   WHEN JSON_TYPE(balance_history) = 'ARRAY' THEN balance_history
                   ELSE JSON_ARRAY()
                 END,
                 JSON_ARRAY()
               ),
               '$',
               ?
             ),
             equity_history = JSON_ARRAY_APPEND(
               COALESCE(
                 CASE 
                   WHEN JSON_TYPE(equity_history) = 'ARRAY' THEN equity_history
                   ELSE JSON_ARRAY()
                 END,
                 JSON_ARRAY()
               ),
               '$',
               ?
             )
         WHERE mt_login = ?`,
        [
          data.current_balance,
          data.current_equity,
          data.floating_pl,
          data.margin_level,
          data.broker,
          data.server,
          data.platform_password,
          data.account_type === 'DEMO' ? 1 : 0,
          now,
          data.current_balance,
          data.current_equity,
          data.mt_login
        ]
      );

      // Verify the update
      const [updated] = await connection.query(
        'SELECT balance_history, equity_history FROM mt_accounts WHERE mt_login = ?',
        [data.mt_login]
      );
      console.log('Updated data:', updated);

      return { ...result, isNewUser: false };
    }
  } catch (error) {
    console.error('Error storing MT data:', error);
    throw error;
  } finally {
    connection.release();
  }
}

// New function to get all MT accounts
async function getAllMTAccounts() {
  const connection = await pool.getConnection();
  try {
    const [rows] = await connection.query(
      `SELECT 
        mt_login,
        order_uid,
        current_balance,
        current_equity,
        floating_pl,
        margin_level,
        broker,
        server,
        is_demo,
        updated_at,
        balance_history,
        equity_history
       FROM mt_accounts 
       ORDER BY updated_at DESC`
    );
    return rows;
  } catch (error) {
    console.error('Error fetching all MT accounts:', error);
    throw error;
  } finally {
    connection.release();
  }
}

// Modify existing GET handler to support both single and all accounts
export async function GET(request: NextRequest) {
  const { searchParams } = new URL(request.url);
  const order_uid = searchParams.get('order_uid');

  try {
    if (order_uid) {
      // Existing logic for single account
      const connection = await pool.getConnection();
      try {
        const [rows] = await connection.query(
          `SELECT * FROM mt_accounts 
           WHERE order_uid = ?
           ORDER BY updated_at DESC 
           LIMIT 1`,
          [order_uid]
        );
        return NextResponse.json((rows as any[])[0] || {});
      } finally {
        connection.release();
      }
    } else {
      // New logic to get all accounts
      const accounts = await getAllMTAccounts();
      return NextResponse.json(accounts);
    }
  } catch (error) {
    console.error('Error in GET /api/mt-data:', error);
    return NextResponse.json(
      { error: 'Failed to fetch MT data' }, 
      { status: 500 }
    );
  }
}

export async function POST(request: Request) {
  try {
    const data = await request.json();
    
    // Validate required fields
    if (!data.mt_login || 
        !data.platform_password ||
        data.current_balance === undefined || 
        data.current_equity === undefined || 
        data.floating_pl === undefined || 
        data.margin_level === undefined) {
      return NextResponse.json({ 
        error: 'Missing required fields',
        required: ['mt_login', 'platform_password', 'current_balance', 'current_equity', 'floating_pl', 'margin_level']
      }, { status: 400 });
    }

    const result = await storeMTData(data);
    
    return NextResponse.json({ 
      message: result.isNewUser ? 'New user created successfully' : 'User data updated successfully',
      mt_login: data.mt_login
    });
  } catch (error) {
    console.error('Error in POST /api/mt-data:', error);
    return NextResponse.json({ 
      error: 'Failed to store MT data',
      details: error instanceof Error ? error.message : 'Unknown error'
    }, { status: 500 });
  }
} 
