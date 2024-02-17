function sendDataToSheet() {
  // Replace 'YOUR_CHANNEL_ID' and 'YOUR_API_KEY' with your actual Thingspeak channel ID and API key
  var channelId = 'XXXX';
  var apiKey = 'XXXX';

  // Replace 'SHEET_NAME' with the name of the sheet you want to send the data to
  var sheetName = 'Sheet1';
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName(sheetName);

  // Get the data from the Thingspeak channel
  var response = UrlFetchApp.fetch('https://api.thingspeak.com/channels/' + channelId + '/feeds.json?api_key=' + apiKey + '&results=4');
  var data = JSON.parse(response.getContentText());
  var feeds = data.feeds;

  // Loop through the feeds and append them to the sheet
  for (var i = 0; i < feeds.length; i++) {
    var feed = feeds[i];
    var date = new Date(feed.created_at);
    var dateString = Utilities.formatDate(date, Session.getScriptTimeZone(), 'dd-MM-yyyy');
    var timeString = Utilities.formatDate(date, Session.getScriptTimeZone(), 'HH:mm:ss');
    var field1 = feed.field1;
    var field2 = feed.field2;

    sheet.appendRow([dateString, timeString, field1, field2]);
  }
}
