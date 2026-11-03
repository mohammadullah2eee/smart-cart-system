/*
  Example Google Apps Script web app for the Smart Cart System.
  It receives the JSON that the ESP32 sends ({"productId": "1", "quantitySold": 1})
  and adds one row to a sheet called "Sales" with the time, product ID and quantity.

  This is an example written for this guide. Your own script can be different.
  Author: Mohammad Ullah (EEE, RUET)   GitHub: https://github.com/mohammadullah2eee/smart-cart-system
*/
function doPost(e) {
  var book = SpreadsheetApp.getActiveSpreadsheet();
  var sheet = book.getSheetByName("Sales") || book.insertSheet("Sales");

  if (sheet.getLastRow() === 0) {
    sheet.appendRow(["Time", "Product ID", "Quantity sold"]);
  }

  var data = JSON.parse(e.postData.contents);
  sheet.appendRow([new Date(), data.productId, data.quantitySold]);

  return ContentService.createTextOutput("OK");
}
