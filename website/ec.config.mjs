import { defineEcConfig } from '@astrojs/starlight/expressive-code';

// Docs is fixed to SlateTeal. A browser/OS light preference must not change code.
export default defineEcConfig({
  themes: ['starlight-dark'],
  useStarlightDarkModeSwitch: false,
  useStarlightUiThemeColors: false,
  useDarkModeMediaQuery: false,
  customizeTheme(theme) {
    theme.bg = '#1b242d';
    theme.fg = '#dee6ec';
    Object.assign(theme.colors, {
      'editor.background': '#1b242d',
      'editor.foreground': '#dee6ec',
      'editorGroupHeader.tabsBackground': '#121920',
      'editorGroupHeader.tabsBorder': '#354958',
      'titleBar.activeBackground': '#121920',
      'titleBar.activeForeground': '#dee6ec',
      'titleBar.border': '#354958',
      'tab.activeBackground': '#1b242d',
      'tab.activeForeground': '#dee6ec',
      'tab.activeBorderTop': '#7cdbd8',
    });
    return theme;
  },
  styleOverrides: {
    borderColor: '#354958',
    borderRadius: '0.4rem',
    frames: {
      editorBackground: '#1b242d',
      editorActiveTabBackground: '#1b242d',
      terminalBackground: '#1b242d',
      inlineButtonForeground: '#dee6ec',
      frameBoxShadowCssValue: 'none',
    },
  },
});
