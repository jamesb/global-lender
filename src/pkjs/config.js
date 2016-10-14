module.exports = [
  {
    "type": "heading",
    "defaultValue": "Global Lender"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "input",
        "messageKey": "LENDER_ID",
        "defaultValue": "",
        "label": "Kiva Lender ID",
        "description": "Not sure? Browse to: http://www.kiva.org/myLenderId",
        "attributes": {
          "placeholder": "eg: jeremy",
        }
      },
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];