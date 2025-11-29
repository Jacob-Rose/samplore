# GitHub Pages Deployment Guide for Samplore

## Quick Setup

### 1. Enable GitHub Pages

1. Go to your repository on GitHub: `https://github.com/Jacob-Rose/samplore`
2. Click on **Settings** (top right)
3. Scroll down and click on **Pages** in the left sidebar
4. Under **Source**, select:
   - Branch: `main` (or your default branch)
   - Folder: `/docs`
5. Click **Save**

GitHub will automatically deploy your site in a few minutes.

### 2. Verify Deployment

Your site will be available at: `https://jacob-rose.github.io/samplore`

You can check the deployment status in the **Actions** tab of your repository.

### 3. Custom Domain (Optional)

If you want to use a custom domain (like `samplore.app` or `www.samplore.app`):

1. In your domain registrar's DNS settings, add a CNAME record:
   ```
   Type: CNAME
   Name: www (or @)
   Value: jacob-rose.github.io
   ```

2. Create a `CNAME` file in the `/docs` directory:
   ```bash
   echo "www.samplore.app" > docs/CNAME
   ```

3. In GitHub repository Settings > Pages, enter your custom domain

4. Enable **Enforce HTTPS** (recommended)

## Testing Locally

To test the site locally before deploying:

### Using Python's HTTP Server

```bash
cd docs
python3 -m http.server 8000
# Open http://localhost:8000 in your browser
```

### Using Node.js http-server

```bash
npm install -g http-server
cd docs
http-server
# Open http://localhost:8080 in your browser
```

## Site Structure

```
docs/
├── index.html          # Main landing page
├── style.css           # Custom styles (no dependencies!)
├── script.js           # Minimal JavaScript
├── .nojekyll          # Tells GitHub Pages to skip Jekyll processing
├── README.md          # Documentation
└── assets/
    └── images/
        ├── logo.png
        ├── screenshot.png
        └── screenshot1.png
```

## Features of This GitHub Pages Site

✅ **Zero dependencies** - No Bootstrap, no jQuery, no Mobirise
✅ **Fully responsive** - Works on mobile, tablet, and desktop
✅ **Fast loading** - Minimal CSS/JS, optimized images
✅ **SEO friendly** - Proper meta tags and semantic HTML
✅ **Accessible** - ARIA labels and semantic structure
✅ **Modern** - Uses CSS Grid, Flexbox, custom properties

## Updating the Site

Simply edit files in the `/docs` directory and push to GitHub:

```bash
cd /home/jakee/Documents/samplore
git add docs/
git commit -m "Update GitHub Pages site"
git push
```

GitHub Pages will automatically rebuild and deploy within a few minutes.

## Customization

### Colors

Edit the CSS variables in `style.css`:

```css
:root {
    --primary: #4A90E2;        /* Main brand color */
    --primary-dark: #357ABD;   /* Darker shade */
    --secondary: #2C3E50;      /* Secondary color */
    --accent: #E74C3C;         /* Accent color */
}
```

### Content

Edit `index.html` directly - it's a simple, self-contained file with clear section comments.

### Images

Replace images in `assets/images/` with your own. Recommended sizes:
- Logo: 148x148px (PNG with transparency)
- Screenshot: 1200x800px or larger

## Troubleshooting

### Site not showing up?

1. Check that GitHub Pages is enabled in Settings > Pages
2. Verify the branch and folder are correct (`main` branch, `/docs` folder)
3. Check the Actions tab for build errors
4. Wait 5-10 minutes after first push

### CSS/JS not loading?

1. Clear your browser cache
2. Check that file paths are relative (no leading `/`)
3. Verify `.nojekyll` file exists

### Images not showing?

1. Check file extensions match (case-sensitive on Linux/GitHub)
2. Verify files exist in `docs/assets/images/`
3. Use browser DevTools to check for 404 errors

## Resources

- [GitHub Pages Documentation](https://docs.github.com/en/pages)
- [HTML Validator](https://validator.w3.org/)
- [CSS Validator](https://jigsaw.w3.org/css-validator/)
