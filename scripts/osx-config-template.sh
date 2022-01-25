# You can find the developer id in your keychain (i guess)
# Team-Id and Asc-Provider are shown with this command: 
# xcrun altool --list-providers -u your@apple-account.com -p your-app-specific-password
DEVELOPER_IDENTITY="Developer ID Application: hansi raber"
DEVELOPER_TEAMID="ABCD12EF"
ASC_PROVIDER="yourname12341234"

# Apple account user (your email address)
NOTARIZE_USER="your@apple-account.com"

# An app specific password (generated in your apple account settings)
NOTARIZE_PASSWORD="your-app-specific-password"
