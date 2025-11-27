# Samplore Website

This directory contains the GitHub Pages site for Samplore.

## Local Development

To test the site locally, you can use any simple HTTP server:

```bash
# Using Python 3
python3 -m http.server 8000

# Using Node.js http-server
npx http-server

# Using PHP
php -S localhost:8000
```

Then visit `http://localhost:8000` in your browser.

## Deployment

This site is automatically deployed to GitHub Pages when changes are pushed to the main branch.

Configure in your repository settings:
- Settings → Pages
- Source: Deploy from a branch
- Branch: main / docs

## Structure

- `index.html` - Main landing page
- `assets/` - All CSS, JS, images, and fonts from the original Mobirise site
