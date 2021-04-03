var fs = require('fs');

fs.copyFile('./dist/mobile/index.html', '../mobile/www/index.html', (err) => {
    if (err) 
        throw err;
    console.log('file was copied to destination');
});