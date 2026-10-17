export const developmentVersion = `${import.meta.env.ROBOT2D_VERSION}-dev`;
export const repo = 'https://github.com/Enziferum/robot2D';
export const path = (route = '') =>
  `${import.meta.env.BASE_URL.replace(/\/$/, '')}/${route.replace(/^\//, '')}`;
export const date = (value: string) =>
  new Intl.DateTimeFormat('en', { dateStyle: 'long', timeZone: 'UTC' }).format(
    new Date(value),
  );
