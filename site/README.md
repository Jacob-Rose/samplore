# Samplore GitHub Pages Site

This directory contains the GitHub Pages website for Samplore.

## Overview

The site is a minimal, dependency-free static website that showcases Samplore's features and provides download links. It was converted from the original Mobirise site to remove dependencies and complete the rebranding from "Samplify" to "Samplore".

## Files

- `index.html` - Main landing page
- `style.css` - Minimal custom CSS (no Bootstrap, no Mobirise dependencies)
- `script.js` - Lightweight JavaScript for smooth scrolling and animations
- `assets/images/` - Logo and screenshots

## Deployment

This site is automatically deployed to GitHub Pages from the `/docs` folder. To enable:

1. Go to repository Settings
2. Navigate to Pages section
3. Set Source to "Deploy from a branch"
4. Select branch: `main` and folder: `/docs`
5. Save

The site will be available at: `https://jacob-rose.github.io/samplore`

## Customization

The site uses CSS custom properties (variables) for easy theming. Edit the `:root` section in `style.css` to change colors:

```css
:root {
    --primary: #4A90E2;
    --secondary: #2C3E50;
    /* etc. */
}
```

## License

Same as the main Samplore project.
