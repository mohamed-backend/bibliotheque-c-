import { GoogleGenAI } from "@google/genai";
import { MediaItem } from "../types";

// Note: In a real production app, never expose API keys on the client side.
// This is for demonstration purposes based on the prompt instructions to use process.env.API_KEY.
const getAIClient = () => {
  if (!process.env.API_KEY) {
    console.warn("API Key is missing. AI features will not work.");
    return null;
  }
  return new GoogleGenAI({ apiKey: process.env.API_KEY });
};

export const askLibrarian = async (
  query: string,
  inventory: MediaItem[],
  history: { role: 'user' | 'model'; text: string }[]
): Promise<string> => {
  const ai = getAIClient();
  if (!ai) return "Error: API Key is not configured.";

  // Create a simplified inventory string to save tokens
  const inventorySummary = inventory.map(item => {
    let details = `${item.titre} (${item.type})`;
    if (!item.dispo) details += " [Unavailable]";
    if (item.type === 'Livre' || item.type === 'Ebook' || item.type === 'AudioBook') {
      details += ` by ${(item as any).auteur}`;
    }
    return details;
  }).join("\n");

  const systemInstruction = `
    You are LibBot, a helpful and knowledgeable AI Librarian for "LibraSys".
    
    Here is the current library catalog:
    ---
    ${inventorySummary}
    ---
    
    Rules:
    1. Answer questions about the catalog (availability, authors, types).
    2. If a user asks for a recommendation, use your general knowledge combined with the catalog to suggest items we actually have.
    3. Be polite and concise.
    4. If the user asks about a book we don't have, politely inform them.
  `;

  try {
    const chat = ai.chats.create({
      model: "gemini-2.5-flash",
      config: {
        systemInstruction,
        temperature: 0.7,
      },
      history: history.map(h => ({
        role: h.role,
        parts: [{ text: h.text }]
      }))
    });

    const result = await chat.sendMessage({ message: query });
    return result.text || "I'm sorry, I couldn't generate a response.";
  } catch (error) {
    console.error("Gemini API Error:", error);
    return "I'm having trouble connecting to the library network right now. Please try again later.";
  }
};
