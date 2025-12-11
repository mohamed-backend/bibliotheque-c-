export type Role = 'Client' | 'Admin' | 'SuperAdmin';

export interface User {
  username: string;
  passwordHash: string;
  role: Role;
}

export type MediaType = 'Livre' | 'Video' | 'Audio' | 'Ebook' | 'AudioBook';

export interface BaseMedia {
  id: number;
  titre: string;
  dispo: boolean;
  type: MediaType;
}

export interface Livre extends BaseMedia {
  type: 'Livre';
  auteur: string;
  nPage: number;
}

export interface Video extends BaseMedia {
  type: 'Video';
  duree: number;
  qualite: string;
}

export interface Audio extends BaseMedia {
  type: 'Audio';
  publicateur: string;
  duree: number;
}

export interface Ebook extends BaseMedia {
  type: 'Ebook';
  auteur: string;
  nPage: number;
  tailleMo: number;
  format: string;
}

export interface AudioBook extends BaseMedia {
  type: 'AudioBook';
  auteur: string;
  nPage: number;
  publicateur: string;
  duree: number;
}

export type MediaItem = Livre | Video | Audio | Ebook | AudioBook;

export interface ChatMessage {
  role: 'user' | 'model';
  text: string;
  timestamp: number;
}
