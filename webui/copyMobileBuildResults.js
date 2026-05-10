var fs = require('fs');

fs.copyFile('./dist/mobile/index.html', '../mobile_flutter/wlanthermo_flutter_app/assets/html/index.html', (err) => {
    if (err) 
        throw err;
    console.log('file was copied to destination');
});