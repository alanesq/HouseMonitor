/**************************************************************************************************
 * 
 *    Send emails from ESP8266/ESP32 via Gmail v2.0 - modified for DadsHouseMonitor sketch (see sendSMS)
 *    
 *    using ESP_Mail_Client library  -  https://github.com/mobizt/ESP-Mail-Client
 *    
 * 
 *    include in main sketch if sending emails is required with command     #include "gmail.h"
 *    
 * 
 * 
 * This demo sketch will fail at the Gmail login unless your Google account has
 * set the following option:     Allow less secure apps: ON       
 *                               see:  https://myaccount.google.com/lesssecureapps
 *
 *                                             26Mar21         
 *  
 **************************************************************************************************

 Usage:

 A good email service to use is:   https://www.gmx.com/mail/

 in main code include:       
                              #include "email.h"
                              // forward declaration
                                void smtpCallback(SMTP_Status status);
                                bool sendEmail(char*, char* , char*);        
                                        

  Using char arrays: // https://www.tutorialspoint.com/arduino/arduino_strings.htm

 
  // send a test email
      _message[0]=0; _subject[0]=0;          // clear any existing text
      strcat(_subject,"test message");
      strcat(_message,"this is a test email from the esp");
      sendEmail(_emailReceiver, _subject, _message);  


 
 ************************************************************************************************** 
 */

//               s e t t i n g s 


  #define _emailReceiver "<email address here>"             // address to send emails
  
  #define _smsReceiver "<email address here>"               // address to send text messages (email to sms gateway)
  
  #define _mailUser "<email address here>"                  // address to send from
  
  #define _mailPassword "<password>"                        // email password

  #define _SMTP "<smtp address>"                            // smtp server address

  #define _SMTP_Port 587                                    // port to use (gmail: Port for SSL: 465, Port for TLS/STARTTLS: 587)

  // #define _SenderName "ShedStation1"                        // name of sender (no spaces)

  #define _UserDomain "gmail.com"                           // user domain to report in email

  const int maxMessageLength = 500;                         // maximum length of email message
  const int maxSubjectLength = 150;                         // maximum length of email subject


//  ----------------------------------------------------------------------------------------


bool sendSMSflag = 0;                                       // if set then also send sms when any email is sent

#include <ESP_Mail_Client.h>  

// stores for email messages
  char _message[maxMessageLength];
  char _subject[maxSubjectLength];
  
// forward declarations
    void smtpCallback(SMTP_Status status);
    bool sendEmail(char*, char* , char*);
  
/* The SMTP Session object used for Email sending */
  SMTPSession smtp;


// ----------------------------------------------------------------------------------------


// Function send an email 
//   see full example: https://github.com/mobizt/ESP-Mail-Client/blob/master/examples/Send_Text/Send_Text.ino


bool sendEmail(char* emailTo, char* emailSubject, char* emailBody) {
 
  if (serialDebug) Serial.println("----- sending an email -------");

  // enable debug info on serial port
    if (serialDebug) { 
      smtp.debug(1);                       // turn debug reporting on
      smtp.callback(smtpCallback);         // Set the callback function to get the sending results
    }

  // Define the session config data which used to store the TCP session configuration
    ESP_Mail_Session session;
  
  // Set the session config
    session.server.host_name =  _SMTP; 
    session.server.port = _SMTP_Port;
    session.login.email = _mailUser;          
    session.login.password = _mailPassword;   
    session.login.user_domain = _UserDomain;

  // Define the SMTP_Message class variable to handle to message being transported
    SMTP_Message message;
    // message.clear();
   
  // Set the message headers
    message.sender.name = _SenderName;
    message.sender.email = _mailUser;
    message.subject = emailSubject;
    message.addRecipient("receiver", emailTo);
    if (sendSMSflag) message.addRecipient("name2", _smsReceiver);
    sendSMSflag = 0;     // clear flag
    // message.addCc("email3");
    // message.addBcc("email4");
  
  // Set the message content
    message.text.content = emailBody;

  // Misc settings
    message.text.charSet = "us-ascii";
    message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_high;
    message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;
    // message.addHeader("Message-ID: <abcde.fghij@gmail.com>");    // custom message header
  
//  // Add attachment to the message
//    message.addAttachment(att);
  
  // Connect to server with the session config
    if (!smtp.connect(&session)) {
      log_system_message("Sending email '" + String(_subject) +"' failed, SMTP: " + smtp.errorReason());
      return 0;
    }
  
  // Start sending Email and close the session
    if (!MailClient.sendMail(&smtp, &message, true)) {
      log_system_message("Sending email '" + String(_subject) +"' failed, Send: " + smtp.errorReason());
      return 0;
    } else {
      log_system_message("Email '" + String(_subject) +"' sent ok");
      return 1;
    }
    
}


// ----------------------------------------------------------------------------------------


/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status) {

  if (!serialDebug) return;
  
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    Serial.println("----------------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      localtime_r(&result.timesstamp, &dt);

      Serial.printf("Message No: %d\n", i + 1);
      Serial.printf("Status: %s\n", result.completed ? "success" : "failed");
      Serial.printf("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      Serial.printf("Recipient: %s\n", result.recipients);
      Serial.printf("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}


// --------------------------- E N D -----------------------------
